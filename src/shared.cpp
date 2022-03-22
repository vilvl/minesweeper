#include "include/shared.hpp"

// sf::Socket::Status receiveWithTimeout(sf::TcpSocket& socket, sf::Packet& packet, sf::Time timeout) {
//     sf::SocketSelector selector;
//     selector.add(socket);
//     if (selector.wait(timeout))
//         return socket.receive(packet);
//     else
//         return sf::Socket::NotReady;
// }
