#include "sf::Font font;
if (!font.loadFromFile("arial.ttf"))
{
    // error...
}shared.hpp"

using namespace std;

void close_each(set<int> SlaveSockets) {
    for (auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) {
        shutdown(*Iter, SHUT_RDWR);
        close(*Iter);
    }
}

void send_each(set<int> SlaveSockets, msg_type type) {
    msg m {.type = type};
    for (auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) {
        send(*Iter, &m, sizeof(msg), MSG_NOSIGNAL);
        // if (send(*Iter, Buffer, RecvSize, MSG_NOSIGNAL) < 0) {
        //     cerr << "ERROR: error while sending data" << endl;
        // }
    }
}

auto connection_closed(set<int>::iterator Iter, set<int> &SlaveSockets) {
    shutdown(*Iter, SHUT_RDWR);
    close(*Iter);
    cout << "Connection closed (id: " << *Iter << ")" << endl;
    auto it = SlaveSockets.erase(Iter);
    send_each(SlaveSockets, CONNECTION_CLOSED);
    return it;
}

void new_connection(int serv_sock, set<int> &SlaveSockets) {
    sockaddr_in cli_addr;
    socklen_t cli_addr_size = sizeof(cli_addr);
    int SlaveSocket = accept(serv_sock, (struct sockaddr*)&cli_addr, &cli_addr_size);
    set_nonblock(SlaveSocket);
    SlaveSockets.insert(SlaveSocket);
    char cli_ip_text[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cli_addr.sin_addr), cli_ip_text, INET_ADDRSTRLEN);
    send_each(SlaveSockets, NEW_CONNECTION);
    cout << "New connection: " << cli_ip_text << ":" << ntohs(cli_addr.sin_port) << " id: " << SlaveSocket << endl;
}

void init_Set(fd_set *Set, int &MasterSocket, set<int> &SlaveSockets) {
    FD_ZERO(Set);
    FD_SET(MasterSocket, Set);
    for (auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++) {
        FD_SET(*Iter, Set);
    }
    int max_fd = max(MasterSocket, *SlaveSockets.rbegin());
    select(max_fd + 1, Set, NULL, NULL, NULL);
}

int main_loop(int &MasterSocket, set<int> &SlaveSockets) {
    fd_set Set;
    init_Set(&Set, MasterSocket, SlaveSockets);
    for (auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); /* Iter++ */) {
        if (FD_ISSET(*Iter, &Set)) {
            static char Buffer[1024];
            int RecvSize = recv(*Iter, Buffer, 1024, MSG_NOSIGNAL);
            Buffer[RecvSize] = '\0';
            if ((RecvSize <= 0) && (errno != EAGAIN)) {
                Iter = connection_closed(Iter, SlaveSockets);
                continue;
            } else {
                cout << Buffer;
                send_each(SlaveSockets, DEFAULT);
            }
        }
        Iter++;
    }
    if (FD_ISSET(MasterSocket, &Set)) {
        new_connection(MasterSocket, SlaveSockets);
    }
    return 0;
}

int main(int argc, char *argv[]) {

    //inet_pton(AF_INET, "10.0.0.1", &(sa.sin_addr)));
    const in_addr_t ip = INADDR_ANY;
    int port;

    if (!(argc == 2 && (port = atoi(argv[1])) && (port > 0) && port <= (MAX_PORT))) {
        cerr << "ERROR: cant get port from cmd argument" << endl;
        return -1;
    }

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MasterSocket < 0) {
        cerr << "ERROR: can't create a socket" << endl;
        return -2;
    }

    set<int> SlaveSockets;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(ip);

    char ip_text[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serv_addr.sin_addr), ip_text, INET_ADDRSTRLEN);

    if (bind(MasterSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
        cerr << "ERROR: can't bind the socket to " << ip_text << ":" << port << endl;
        return -2;
    }
    set_nonblock(MasterSocket);
    if (listen(MasterSocket, SOMAXCONN)) {
        cerr << "ERROR: the socket can't listen" << endl;
        return -2;
    }

    while (main_loop(MasterSocket, SlaveSockets) == 0) {}

    shutdown(MasterSocket, SHUT_RDWR);
    close(MasterSocket);
}
