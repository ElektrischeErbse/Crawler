//
// Created by tom on 26-4-14.
//

#include <requests/requests.h>
#include <gtest/gtest.h>
#include <fstream>

TEST(SessionTest, GetRequest)
{
    Session sess;
    auto res = sess.get("https://httpbin.org/get");
    EXPECT_EQ(res.status_code, 200);
    EXPECT_TRUE(res.ok());
    EXPECT_FALSE(res.text.empty());
}

TEST(SessionTest, PostRawBody)
{
    Session sess;
    auto res = sess.post("https://httpbin.org/post", "hello=world");
    EXPECT_EQ(res.status_code, 200);
    EXPECT_TRUE(res.ok());
}

TEST(SessionTest, PostJson)
{
    Session sess;
    std::string json = R"({"name":"test","age":20})";
    auto res = sess.post_json("https://httpbin.org/post", json);
    EXPECT_EQ(res.status_code, 200);
    EXPECT_TRUE(res.text.find("test") != std::string::npos);
}

TEST(SessionTest, PostForm)
{
    Session sess;
    std::map<std::string, std::string> form = {
        {"username", "tom"},
        {"password", "123456"}
    };
    auto res = sess.post_form("https://httpbin.org/post", form);
    EXPECT_EQ(res.status_code, 200);
    EXPECT_TRUE(res.text.find("tom") != std::string::npos);
}

TEST(SessionTest, PutRequest)
{
    Session sess;
    auto res = sess.put("https://httpbin.org/put", "data=123");
    EXPECT_EQ(res.status_code, 200);
}

TEST(SessionTest, DeleteRequest)
{
    Session sess;
    auto res = sess.del("https://httpbin.org/delete");
    EXPECT_EQ(res.status_code, 200);
}

TEST(SessionTest, DownloadFile)
{
    Session sess;
    // 下载一个小测试文件到 WSL 临时路径
    bool ok = sess.download("https://httpbin.org/image/jpeg", "/tmp/test.jpg");
    EXPECT_TRUE(ok);
}

TEST(SessionTest, UploadFile)
{
    // 先创建一个临时文件
    std::ofstream tmpfile("/tmp/test_upload.txt");
    tmpfile << "test content";
    tmpfile.close();

    Session sess;
    auto res = sess.upload("https://httpbin.org/post", "file", "/tmp/test_upload.txt");
    EXPECT_EQ(res.status_code, 200);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}