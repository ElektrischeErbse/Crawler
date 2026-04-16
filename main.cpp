//
// Created by tom on 26-4-15.
//

#include <iostream>
#include <requests/requests.h>
#include <gq/Document.h>
#include <gq/Node.h>

using namespace std;

int main()
{
    Session s;
    auto r = s.get("https://www.baidu.com");
    if(r.ok()){
        // 解析html
        CDocument doc;
        doc.parse(r.text);
        CSelection selection = doc.find("#head_wrapper #lg img");
        auto img_src = selection.nodeAt(0).attribute("src");
        cout << img_src << endl;
        // 提取后缀
        auto idx = img_src.rfind('.');
        if(idx == string::npos){
            return 1;
        }
        string ext_name = img_src.substr(idx);
        cout << ext_name << endl;
        // 下载logo图片
        string url = "https:" + img_src;
        string filename = "logo" + ext_name;
        if(!s.download(url, filename)){
            cout << "download logo failed" << endl;
        }
    }
    return 0;
}