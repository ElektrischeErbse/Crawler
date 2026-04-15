//
// Created by tom on 26-4-15.
//

#ifndef CRAWLER_REQUESTS_H
#define CRAWLER_REQUESTS_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

// 响应对象
struct Response {
    int status_code = 0;
    std::string text;
    std::string url;
    std::map<std::string, std::string> headers;
    std::string error;

    bool ok() const;
    json to_json() const;
};

class Session {
public:
    Session();
    ~Session();

    Session &timeout(int seconds);
    Session &headers(const std::map<std::string, std::string> &headers);
    Session &add_header(const std::string &key, const std::string &value);
    Session &verify(bool enable);
    Session &proxy(const std::string &proxy);
    Session &follow_redirects(bool enable);

    // 请求
    Response get(const std::string &url);
    Response post(const std::string &url, const std::string &body);
    Response post_json(const std::string &url, const std::string &json);
    Response post_form(const std::string &url, const std::map<std::string, std::string> &form);
    Response put(const std::string &url, const std::string &body);
    Response del(const std::string &url);

    // 文件上传和下载
    Response upload(const std::string &url, const std::string &field, const std::string &filepath);
    bool download(const std::string &url, const std::string &save_path);

    void clear_headers();
private:
    void prepare_common(const std::string &url);
    Response perform();

    CURL *curl_ = nullptr;
    curl_slist *slist_ = nullptr;
    std::map<std::string, std::string> default_headers_;
    long timeout_ = 30;
    bool verify_ssl_ = false;
    bool follow_redirect_ = true;
    std::string proxy_;
};

Response get(const std::string &url);
Response post(const std::string &url, const std::string &body);
Response post_json(const std::string &url, const std::string &json);
Response post_form(const std::string &url, const std::map<std::string, std::string> &form);
bool download(const std::string &url, const std::string &save_path);


#endif //CRAWLER_REQUESTS_H
