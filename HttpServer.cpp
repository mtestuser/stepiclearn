
#include "HttpServer.h"
#include <iostream>
#include <thread>
#include <sys/stat.h>

using namespace std;

void HttpServer::start() {
    struct sockaddr_in addr;
    list = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(list < 0){
        cout << "Server socket create error" << endl;
        exit(1);
    }

    addr.sin_family = AF_INET;
    cout << "port : " << opt_.port << endl;
    addr.sin_port = htons(opt_.port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(list, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        cout << "Can't bind to the port " << opt_.port << endl;
        exit(1);
    }

    cout << "Server start" << endl;
    listen(list, SOMAXCONN);

    int cliSock = 0;
    while(1)
    {
        cliSock = accept(list, NULL, NULL);
        if(cliSock < 0){
            cout << "Accept error" << endl;
            exit(1);
        }

        //thread thr(clientMethod, cliSock);
        thread thr(&HttpServer::newClientRecvThr, this, cliSock);
        thr.detach();
    }
}

//Запускаем обработку запроса клиента в новом потоке
void HttpServer::newClientRecvThr(int socket) {
    char *buffer = new char[8096];
    int n = recv(socket, buffer, 8096, 0);

    if(n <= 0){
        return;
    }

    string page(opt_.dir);
    page += parseRequest(buffer);

    string http = genResponse(page);

    send(socket, http.c_str(), http.size(), 0);

    return;
}

//Парсим запрос на запрашиваемый файл
string HttpServer::parseRequest(char *buffer) {
    string s(buffer);
    string out;
    if(s.find("GET") == 0){
        out = s.substr(3, s.find("HTTP") - 3);
        size_t first = out.find_first_not_of(' ');
        size_t last = out.find_last_not_of(' ');
		if(first == last){
            out = "";
        }
		else{
        out = out.substr(first, (last-first+1));
		}
    }
    return  out;
}

//Формирование страницы html
string HttpServer::genResponse(string &page) {
    struct stat st;
    bool exists = (stat(page.c_str(), &st) == 0);
    if(exists)
    {
        exists = st.st_mode & S_IFDIR;
    }

    FILE *file = fopen(page.c_str(), "r");

    if(!exists && file != NULL){
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file,0,SEEK_SET);
        char *buffer = new char[size];
        fread(buffer, 1, size, file);
        fclose(file);

        string out = "HTTP/1.0 200 OK\r\n" \
                    "Content-type: text/html\r\n" \
                    "Content-length: " + to_string(size) + "\r\n\r\n";
        out.append(buffer, size);

        return out;
    }
    else{
        string notFound = "HTTP/1.0 404 Not Found\r\n" \
                    "Content-type: text/html\r\n" \
                    "Content-length: 46\r\n" \
                    "\r\n\r\n" \
                    "<html><body><p>404 not fount</p></body></html>";
        return notFound;
    }
}
