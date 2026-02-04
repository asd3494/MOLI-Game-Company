#include <httplib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

int main() {
	httplib::Server svr;
	
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
	if (!svr.listen("localhost", 8080)) {
		cerr << "错误：端口 8080 可能已被占用！" << endl;
		cerr << "请尝试：" << endl;
		cerr << "1. 关闭占用该端口的程序" << endl;
		cerr << "2. 使用其他端口号" << endl;
		cerr << "3. 以管理员/root权限运行" << endl;
		return 1;
	}
	
	return 0;
}

