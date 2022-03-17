#include "client_socket.hpp"

// using namespace std;

int parse_cmd_args(int argc, char *argv[], char *&host, int &port) {
    if (!(argc == 3
            && (host = argv[1])
            && (port = atoi(argv[2])) && (port > 0) && port <= (MAX_PORT))) {
        std::cerr << "ERROR: cmd arguments should be host-address and port separated with space" << std::endl;
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char* host;
    int port;

    if (parse_cmd_args(argc, argv, host, port) == -1)
        exit(-1);

    int sock = get_client_socket(host, port);
    if (sock == -1)
        exit(-2);

    char buf_send[1024], buf_recv[1024];
    do {
        fd_set Set;
        FD_ZERO(&Set);
        FD_SET(sock, &Set);
        FD_SET(STDIN_FILENO, &Set);
        int max_fd = std::max(sock, STDIN_FILENO);
        select(max_fd + 1, &Set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &Set)) {
            char* pnt = fgets(buf_send, 1024, stdin);
            send(sock, pnt, strlen(pnt), MSG_NOSIGNAL);
        }
        if (FD_ISSET(sock, &Set)) {
            int b = recv(sock, buf_recv, 1024, MSG_NOSIGNAL);
            if (b <= 0)
                break;
            buf_recv[b] = '\0';
            std::cout << buf_recv;
        }
    } while (strcmp(buf_send, "stop\n"));

    shutdown(sock, SHUT_RDWR);
    close(sock);
}