#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <vector>

using namespace std;

bool findUserInFile(const string& username, const string& password) {
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
			
			if (file_user == username && file_pass == password) {
				file.close();
				return true;
			}
		}
	}
	file.close();
	return false;
}

int main() {
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
				res.set_content(
								"{\"status\": \"error\", \"message\": \"Username and password cannot be empty\"}",
								"application/json"
								);
				cout << "[LOG] Error: Empty username or password" << endl;
				return;
			}
			
			ofstream file("userlist.txt", ios::app);
			if (!file.is_open()) {
				res.status = 500;
				res.set_content(
								"{\"status\": \"error\", \"message\": \"Server error: cannot open user database\"}",
								"application/json"
								);
				cerr << "Error: Cannot open userlist.txt for writing" << endl;
				return;
			}
			
			file << username << ":" << password << endl;
			file.close();
			
			cout << "[LOG] User registered: " << username << endl;
			
			res.set_content(
							"{\"status\": \"success\", \"message\": \"Registration successful\", \"redirect\": \"/auth/login\"}",
							"application/json"
							);
			
		} else {
			res.status = 400;
			res.set_content(
							"{\"status\": \"error\", \"message\": \"Data format should be: username:password\"}",
							"application/json"
							);
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
				res.set_content(
								"{\"status\": \"success\", \"message\": \"Login successful\", \"redirect\": \"/main\"}",
								"application/json"
								);
				cout << "[LOG] Login successful, user: " << username << endl;
			} else {
				res.set_content(
								"{\"status\": \"error\", \"message\": \"Incorrect ID or password\"}",
								"application/json"
								);
				cout << "[LOG] Login failed, user: " << username << endl;
			}
		} else {
			res.status = 400;
			res.set_content(
							"{\"status\": \"error\", \"message\": \"Data format should be: ID:password\"}",
							"application/json"
							);
			cout << "[LOG] Error: Invalid request body format" << endl;
		}
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
