//
// Created by tom on 26-4-15.
//

#include <requests/requests.h>
#include <curl/curl.h>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <fstream>

bool Response::ok() const
{
    return status_code >= 200 && status_code < 300;
}

json Response::to_json() const
{
    return json::parse(text);
}

Session::Session()
{
    curl_ = curl_easy_init();
    if (!curl_) {
        throw std::runtime_error("curl_easy_init failed");
    }
}

Session::~Session()
{
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

Session &Session::timeout(int seconds)
{
    timeout_ = seconds;
    return *this;
}

Session &Session::headers(const std::map<std::string, std::string> &headers)
{
    default_headers_ = headers;
    return *this;
}

Session &Session::add_header(const std::string &key, const std::string &value)
{
    default_headers_[key] = value;
    return *this;
}

Session &Session::verify(bool enable)
{
    verify_ssl_ = enable;
    return *this;
}

Session &Session::proxy(const std::string &proxy)
{
    proxy_ = proxy;
    return *this;
}

Session &Session::follow_redirects(bool enable)
{
    follow_redirect_ = enable;
    return *this;
}

void Session::prepare_common(const std::string &url)
{
    // 重置设置
    curl_easy_reset(curl_);
    // 设置URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    // 设置超时
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, timeout_);
    // 设置SSL验证
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);
    // 设置自动重定向
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, follow_redirect_ ? 1L : 0L);
    // 设置代理
    if (!proxy_.empty()) {
        curl_easy_setopt(curl_, CURLOPT_PROXY, proxy_.c_str());
    }
    // 设置头部
    curl_slist *slist = nullptr;
    for (auto &[key, value]: default_headers_) {
        std::string header;
        header.append(key);
        header.append(": ");
        header.append(value);
        slist = curl_slist_append(slist, header.c_str());
    }
    if (slist) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, slist);
        curl_slist_free_all(slist);
    }
}

// 数据回调
static size_t write_callback(char *contents, size_t data_block_size, size_t data_block_count, void *user_data)
{
    size_t total = data_block_size * data_block_count;
    auto s = reinterpret_cast<std::string *>(user_data);
    s->append(contents, total);
    return total;
}

// 下载文件回调
static size_t file_write_callback(void *contents, size_t data_block_size, size_t data_block_count, void *user_data)
{
    size_t total = data_block_size * data_block_count;
    auto &save_file = *reinterpret_cast<std::ofstream *>(user_data);
    save_file.write(reinterpret_cast<char *>(contents), total);
    return total;
}

// 响应头解析
static size_t header_callback(char *contents, size_t data_block_size, size_t data_block_count, void *user_data)
{
    size_t len = data_block_size * data_block_count;
    std::string line(contents, len);
    auto pos = line.find(':');
    if (pos == std::string::npos) {
        return len;
    }

    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 1);

    // 移除前后空白符
    auto trim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !isspace(ch);
        }).base(), s.end());
    };
    trim(key);
    trim(val);

    auto *headers = reinterpret_cast<std::map<std::string, std::string> *>(user_data);
    headers->insert_or_assign(key, val);
    return len;
}

Response Session::perform()
{
    Response response;
    // 设置body回调函数
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    // 设置body回调函数保存数据指针
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.text);
    // 设置header回调函数
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, header_callback);
    // 设置header回调函数保存数据指针
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response.headers);

    CURLcode code = curl_easy_perform(curl_);
    if (code != CURLE_OK) {
        response.error = curl_easy_strerror(code);
        return response;
    }

    long http_code;
    code = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    response.status_code = static_cast<int>(http_code);
    char *url = nullptr;
    code = curl_easy_getinfo(curl_, CURLINFO_EFFECTIVE_URL, &url);
    if (url) {
        response.url = url;
    }
    return response;
}

Response Session::get(const std::string &url)
{
    prepare_common(url);
    return perform();
}

Response Session::post(const std::string &url, const std::string &body)
{
    prepare_common(url);
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, (long) body.size());
    return perform();
}

Response Session::post_json(const std::string &url, const std::string &json)
{
    add_header("Content-Type", "application/json");
    auto r = post(url, json);
    // 移除 Content-Type 便于复用
    default_headers_.erase("Content-Type");
    return r;
}

Response Session::post_form(const std::string &url, const std::map<std::string, std::string> &form)
{
    std::string data;
    // name=tom&age=18
    for (auto &[k, v]: form) {
        if (!data.empty()) {
            data += '&';
        }
        data.append(k);
        data.append("=");
        data.append(v);
    }
    add_header("Content-Type", "application/x-www-form-urlencoded");
    auto r = post(url, data);
    default_headers_.erase("Content-Type");
    return r;
}

Response Session::put(const std::string &url, const std::string &body)
{
    prepare_common(url);
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
    return perform();
}

Response Session::del(const std::string &url)
{
    prepare_common(url);
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    return perform();
}

Response Session::upload(const std::string &url, const std::string &field, const std::string &filepath)
{
    // upload(url, "file", "xxx.jpg")
    prepare_common(url);
    curl_mime* mime = curl_mime_init(curl_);
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_name(part, field.c_str());
    curl_mime_filedata(part, filepath.c_str());
    curl_easy_setopt(curl_, CURLOPT_MIMEPOST, mime);
    auto res = perform();
    curl_mime_free(mime);
    return res;
}

bool Session::download(const std::string &url, const std::string &save_path)
{
    std::ofstream save_file(save_path, std::ios::binary);
    if(!save_file.is_open()){
        return false;
    }
    prepare_common(url);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, file_write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &save_file);
    CURLcode res = curl_easy_perform(curl_);
    return res == CURLE_OK;
}

void Session::clear_headers()
{
    default_headers_.clear();
}

Response get(const std::string &url)
{
    Session s;
    return s.get(url);
}

Response post(const std::string &url, const std::string &body)
{
    Session s;
    return s.post(url, body);
}

Response post_json(const std::string &url, const std::string &json)
{
    Session s;
    return s.post_json(url, json);
}

Response post_form(const std::string &url, const std::map<std::string, std::string> &form)
{
    Session s;
    return s.post_form(url, form);
}

bool download(const std::string &url, const std::string &save_path)
{
    Session s;
    return s.download(url, save_path);
}