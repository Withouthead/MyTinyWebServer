//
// Created by xiaomaotou31 on 2021/11/13.
//


#include "../HttpUtils/HttpRequest.h"
#include "../HttpUtils/HttpConnect.h"
#include "../HttpUtils/HttpResponse.h"
#include <iostream>
#include <sys/epoll.h>

using namespace std;

int main(int argc, char *argv[]) {
    unordered_map<int, HttpConnect> httpConnects;

    int user_num = 0;
    int sockfd;
    sockaddr_in server_addr{};
    memset(&server_addr, 0, sizeof(server_addr));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int r = bind(sockfd, (sockaddr *) (&server_addr), sizeof(server_addr));
    assert(r >= 0);
    r = listen(sockfd, 10);
    assert(r >= 0);


    int epollfd = epoll_create(5);
    epoll_event event{};
    event.data.fd = sockfd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);

    epoll_event events[10000];
    while (1) {
        cout << "waiting connection !" << endl;
        int num = epoll_wait(epollfd, events, 100, -1);
        for (int i = 0; i < num; i++) {
            int fd = events[i].data.fd;
            if (fd == sockfd) {
                cout << "new connection!" << endl;
                sockaddr_in client_addr{};
                socklen_t addr_size = 0;
                int client_fd = accept(sockfd, (sockaddr *) &client_addr, &addr_size);
                if (client_fd < 0)
                    continue;

                memset(&event, 0, sizeof(event));
                event.data.fd = client_fd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &event);
                httpConnects.insert({client_fd, HttpConnect(client_fd, client_addr, false, "./root")});

            } else if (events[i].events & EPOLLIN) {
                HttpConnect &httpConnect = httpConnects.find(fd)->second;
                httpConnect.Read();
                if (!httpConnect.Process()) {
                    cout << "Process Error" << endl;
                    return 1;
                }
                httpConnect.Write();
                httpConnect.Close();
            }


        }

    }

    close(sockfd);
    return 0;
}