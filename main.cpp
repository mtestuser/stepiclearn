#include <iostream>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "HttpServer.h"

using namespace std;

bool parseOptions(int argc, char **argv, struct Options *opt);

//main
int main(int argc, char **argv) {

    if(argc < 7)
    {
        cout << "Low param count" << endl;
        return 1;
    }


    struct Options opt;
    if(parseOptions(argc, argv, &opt) == false)
    {
        return 1;
    }

    pid_t pid = fork();

    //Ошибка создания потомка
    if(pid == -1)
    {
        cout << "Start daemon error" << endl;
    }
    else if(!pid)
    {
        //Сохраняем pid потомка
        pid_t child_pid = getpid();

        char str[8];
        int n = sprintf(str, "%d", child_pid);

        FILE *pid_file = fopen("pid", "w");
        fwrite(str, 1, n, pid_file);
        fclose(pid_file);

        umask(0);

        // создаём новый сеанс
        setsid();

        // переходим в корень диска
        chdir("/");

        //Закрываем дескрипторы ввода/вывода
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        //Поднимаем http сервер
        HttpServer serv(opt);
        serv.start();
    }

    return 0;
}

//Обработка параметров
bool parseOptions(int argc, char **argv, struct Options *opt)
{
    int res = 0;
    bool ret = false;
    int port = 0;

    while((res = getopt(argc, argv, "hpd")) != -1)
    {
        switch (res)
        {
            case 'h':
                if(inet_aton(argv[optind], &opt->host_ip) == 0)
                {
                    cout << "Wrong parametr -h: " << argv[optind] << endl;
                    return ret;
                }
                break;
            case 'p':
                port = atoi(argv[optind]);
                if(port > 65535 || port < 0)
                {
                    cout << "Wrong parametr -p: " << port << endl;
                    return ret;
                }
                else
                {
                    opt->port = port;
                }
                break;
            case 'd':
                if(argv[optind] == NULL)
                {
                    cout << "Parametr -d can't be NULL" << endl;
                    return ret;
                }
                opt->dir = argv[optind];
                break;
            case '?':
                cout << "Parametr error." << endl;
                return ret;
                break;
        }
    }

    return true;
}