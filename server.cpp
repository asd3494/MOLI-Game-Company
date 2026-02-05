#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <set>

using namespace std;

// 全局变量存储已注册的用户，用于快速查找
set<string> registeredUsers;

// 加载已注册用户到内存
void loadRegisteredUsers() {
    ifstream file("userlist.txt");
    if (!file.is_open()) {
        cout << "[INFO] userlist.txt not found, starting with empty user list" << endl;
        return;
    }
    
    string line;
    while (getline(file, line)) {
        size_t colon_pos = line.find(":");
        if (colon_pos != string::npos && colon_pos > 0) {
            string username = line.substr(0, colon_pos);
            username.erase(username.find_last_not_of(" \n\r\t") + 1);
            if (!username.empty()) {
                registeredUsers.insert(username);
                cout << "[LOAD] Loaded user: " << username << endl;
            }
        }
    }
    file.close();
    cout << "[INFO] Total " << registeredUsers.size() << " users loaded" << endl;
}

bool isUIDAllowed(const string& uid) {
    ifstream file("allow.txt");
    if (!file.is_open()) {
        cerr << "Warning: Cannot open allow.txt" << endl;
        return false;
    }
    string line;
    while (getline(file, line)) {
        line.erase(line.find_last_not_of(" \n\r\t") + 1);
        if (line == uid) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool findUserInFile(const string& username, const string& password) {
    if (!isUIDAllowed(username)) {
        return false;
    }
    
    ifstream file("userlist.txt");
    if (!file.is_open()) {
        return false;
    }
    string line;
    while (getline(file, line)) {
        size_t colon_pos = line.find(":");
        if (colon_pos != string::npos) {
            string file_user = line.substr(0, colon_pos);
            string file_pass = line.substr(colon_pos + 1);
            
            // 去除空白字符
            file_user.erase(file_user.find_last_not_of(" \n\r\t") + 1);
            file_pass.erase(file_pass.find_last_not_of(" \n\r\t") + 1);
            
            if (file_user == username && file_pass == password) {
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}

// 辅助函数：从URL路径中提取用户名
string extractUsernameFromPath(const string& path) {
    // 路径格式应该是: /app/users/check/username
    size_t last_slash = path.find_last_of('/');
    if (last_slash != string::npos) {
        return path.substr(last_slash + 1);
    }
    return "";
}

int main() {
    // 启动时加载已注册用户
    loadRegisteredUsers();
    
    httplib::Server svr;
    
    svr.Options(".*", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Max-Age", "86400");
    });
    
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        if (req.method == "OPTIONS") {
            res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });
    
    svr.Get("/main", [](const httplib::Request& req, httplib::Response& res) {
        ifstream file("index.html");
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File 'index.html' not found!", "text/plain");
            cerr << "Error: Cannot open index.html" << endl;
            return;
        }
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "text/html");
        cout << "Successfully served index.html" << endl;
    });
    
    svr.Get("/auth/login", [](const httplib::Request& req, httplib::Response& res){
        ifstream file("login_page.html");
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File 'login_page.html' not found!", "text/plain");
            cerr << "Error: Cannot open login_page.html" << endl;
            return;
        }
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "text/html");
        cout << "Successfully served login_page.html" << endl;
    });
    
    svr.Get("/auth/signup", [](const httplib::Request& req, httplib::Response& res){
        ifstream file("signup_page.html");
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File 'signup_page.html' not found!", "text/plain");
            cerr << "Error: Cannot open signup_page.html" << endl;
            return;
        }
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "text/html");
        cout << "Successfully served signup_page.html" << endl;
    });
    
    svr.Get("/auth/uid", [](const httplib::Request& req, httplib::Response& res) {
        string auth_header = req.get_header_value("Authorization");
        
        if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }
        
        string token = auth_header.substr(7);
        token.erase(token.find_last_not_of(" \n\r\t") + 1);
        
        res.set_content(token, "text/plain");
        cout << "[AUTH] UID retrieved: " << token << endl;
    });
    
    // 提供app/userlist.txt的访问
    svr.Get("/app/userlist.txt", [](const httplib::Request& req, httplib::Response& res) {
        ifstream file("userlist.txt");
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File 'userlist.txt' not found!", "text/plain");
            cerr << "Error: Cannot open userlist.txt for reading" << endl;
            return;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        res.set_content(buffer.str(), "text/plain");
        res.set_header("Content-Type", "text/plain; charset=utf-8");
        cout << "[FILE] userlist.txt served" << endl;
    });
    
    svr.Post("/app/new-user/register", [](const httplib::Request& req, httplib::Response& res) {
        cout << "[LOG] Received registration request" << endl;
        string txt_body = req.body;
        cout << "[LOG] Request body: " << txt_body << endl;
        
        size_t colon_pos = txt_body.find(":");
        if (colon_pos != string::npos && colon_pos > 0) {
            string username = txt_body.substr(0, colon_pos);
            string password = txt_body.substr(colon_pos + 1);
            
            username.erase(username.find_last_not_of(" \n\r\t") + 1);
            password.erase(password.find_last_not_of(" \n\r\t") + 1);
            
            if (username.empty() || password.empty()) {
                res.status = 400;
                res.set_content("用户名和密码不能为空", "text/plain");
                cout << "[LOG] Error: Empty username or password" << endl;
                return;
            }
            
            // 检查UID是否已经在allow.txt中允许
            if (!isUIDAllowed(username)) {
                res.status = 403;
                res.set_content("此UID未授权注册", "text/plain");
                cout << "[LOG] Registration rejected: UID not in allow.txt - " << username << endl;
                return;
            }
            
            // 检查用户名是否已存在
            if (registeredUsers.find(username) != registeredUsers.end()) {
                res.status = 409; // HTTP 409 Conflict
                res.set_content("用户ID已存在", "text/plain");
                cout << "[LOG] Registration rejected: User already exists - " << username << endl;
                return;
            }
            
            // 检查文件中的重复（双重检查）
            ifstream checkFile("userlist.txt");
            if (checkFile.is_open()) {
                string line;
                while (getline(checkFile, line)) {
                    size_t pos = line.find(":");
                    if (pos != string::npos) {
                        string existing_user = line.substr(0, pos);
                        existing_user.erase(existing_user.find_last_not_of(" \n\r\t") + 1);
                        if (existing_user == username) {
                            checkFile.close();
                            res.status = 409;
                            res.set_content("用户ID已存在", "text/plain");
                            cout << "[LOG] Registration rejected: User found in file - " << username << endl;
                            registeredUsers.insert(username); // 添加到内存缓存
                            return;
                        }
                    }
                }
                checkFile.close();
            }
            
            ofstream file("userlist.txt", ios::app);
            if (!file.is_open()) {
                res.status = 500;
                res.set_content("服务器错误：无法打开用户数据库", "text/plain");
                cerr << "Error: Cannot open userlist.txt for writing" << endl;
                return;
            }
            
            file << username << ":" << password << endl;
            file.close();
            
            // 添加到内存缓存
            registeredUsers.insert(username);
            
            cout << "[LOG] User registered successfully: " << username << endl;
            cout << "[LOG] Total registered users: " << registeredUsers.size() << endl;
            
            res.set_content("注册成功", "text/plain");
            
        } else {
            res.status = 400;
            res.set_content("数据格式应为：用户名:密码", "text/plain");
            cout << "[LOG] Error: Invalid request body format" << endl;
        }
    });
    
    svr.Post("/app/new-user/json", [](const httplib::Request& req, httplib::Response& res) {
        cout << "[LOG] Received login request" << endl;
        string txt_body = req.body;
        cout << "[LOG] Request body: " << txt_body << endl;
        
        size_t colon_pos = txt_body.find(":");
        if (colon_pos != string::npos && colon_pos > 0) {
            string username = txt_body.substr(0, colon_pos);
            string password = txt_body.substr(colon_pos + 1);
            
            username.erase(username.find_last_not_of(" \n\r\t") + 1);
            password.erase(password.find_last_not_of(" \n\r\t") + 1);
            
            cout << "[LOG] Parsed - ID: \"" << username << "\", Password: \"" << password << "\"" << endl;
            
            bool login_success = findUserInFile(username, password);
            
            if (login_success) {
                res.set_content(username, "text/plain");
                cout << "[LOG] Login successful, user: " << username << endl;
            } else {
                if (!isUIDAllowed(username)) {
                    res.status = 403;
                    res.set_content("此UID未授权登录", "text/plain");
                    cout << "[LOG] Login rejected: UID not in allow.txt - " << username << endl;
                } else {
                    res.status = 401;
                    res.set_content("用户名或密码错误", "text/plain");
                    cout << "[LOG] Login failed (wrong password), user: " << username << endl;
                }
            }
        } else {
            res.status = 400;
            res.set_content("数据格式应为：用户名:密码", "text/plain");
            cout << "[LOG] Error: Invalid request body format" << endl;
        }
    });
    
    // 新增：获取已注册用户列表API
    svr.Get("/app/users/list", [](const httplib::Request& req, httplib::Response& res) {
        ostringstream oss;
        oss << "已注册用户数量: " << registeredUsers.size() << "\n\n";
        
        for (const auto& user : registeredUsers) {
            oss << user << "\n";
        }
        
        res.set_content(oss.str(), "text/plain");
        cout << "[API] User list requested, returned " << registeredUsers.size() << " users" << endl;
    });
    
    // 新增：检查用户名是否可用 - httplib 0.8.6兼容版本
    // 使用查询参数方式: GET /app/users/check?username=testuser
    svr.Get("/app/users/check", [](const httplib::Request& req, httplib::Response& res) {
        // 在httplib 0.8.6中，使用req.get_param_value获取查询参数
        string username;
        
        // 方式1：使用get_param_value
        if (req.has_param("username")) {
            username = req.get_param_value("username");
        }
        // 方式2：也可以从URL中解析
        else {
            // 尝试从路径解析：/app/users/check/username
            username = extractUsernameFromPath(req.path);
        }
        
        if (username.empty()) {
            res.status = 400;
            res.set_content("请提供用户名参数", "text/plain");
            return;
        }
        
        // 输出调试信息
        cout << "[CHECK] Checking username: " << username << endl;
        cout << "[CHECK] Path: " << req.path << endl;
        cout << "[CHECK] Has param 'username': " << (req.has_param("username") ? "true" : "false") << endl;
        
        if (registeredUsers.find(username) != registeredUsers.end()) {
            res.set_content("unavailable", "text/plain");
            cout << "[CHECK] Username check: " << username << " is unavailable" << endl;
        } else {
            res.set_content("available", "text/plain");
            cout << "[CHECK] Username check: " << username << " is available" << endl;
        }
    });
    
    // 添加一个简单的路由处理器来测试参数
    svr.Get("/test/params", [](const httplib::Request& req, httplib::Response& res) {
        ostringstream oss;
        oss << "Path: " << req.path << "\n";
        oss << "Query string: " << req.body << "\n"; // 注意：在0.8.6中，查询参数在req.body中
        
        // 输出所有头部信息用于调试
        oss << "\nHeaders:\n";
        for (const auto& header : req.headers) {
            oss << header.first << ": " << header.second << "\n";
        }
        
        res.set_content(oss.str(), "text/plain");
    });
    
    int port = 8080;
    if (const char* env_port = std::getenv("PORT")) {
        port = std::stoi(env_port);
    }
    if (!svr.listen("0.0.0.0", port)) {
        cerr << "Error: Port " << port << " may be occupied!" << endl;
        return 1;
    }
    
    return 0;
}
