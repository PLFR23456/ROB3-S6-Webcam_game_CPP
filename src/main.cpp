#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "camera.h"

int main() {
    camera();
    try {
        boost::asio::io_context io;
        boost::asio::serial_port serial(io);

        // Ouvre le port série
        serial.open("/dev/ttyACM0"); 
    
        // Configure le port
        serial.set_option(boost::asio::serial_port_base::baud_rate(9600));
        serial.set_option(boost::asio::serial_port_base::character_size(8));
        serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    
        std::string message;
        while (true) {
            std::cout << "Entrez un message à envoyer (ou 'exit' pour quitter) : ";
            std::getline(std::cin, message);

            if (message == "exit") {
                std::cout << "Fermeture du programme." << std::endl;
                break;
            }

            message += "\n"; // Ajoute un saut de ligne pour respecter le format attendu
            std::cout << "Envoi de : " << message;
            auto bytes = boost::asio::write(serial, boost::asio::buffer(message)); 
            std::cout << "Octets envoyés : " << bytes << std::endl;
        }

        serial.close();
    } catch (std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }
       
    return 0;
}