
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <vector>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



#define H_PORT 53884
#define M_UDP_PORT 54884
#define TRUE 1
#define FALSE 0

using namespace std;

struct sockaddr_in addrH, udp_AddrM;

int main(){
    // backend server boot session

    //setting up UDP server
    int udpSkt = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSkt == -1){
        perror("Creating UDP error");
        exit(-1);
    }

    //udp sockets configuration
    udp_AddrM.sin_family = AF_INET;
    udp_AddrM.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_AddrM.sin_port = htons(M_UDP_PORT);

    addrH.sin_family = AF_INET;
    addrH.sin_port = htons(H_PORT);
    addrH.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(udpSkt, (struct sockaddr*)&addrH, sizeof(addrH)) == -1){
        perror("Binding UDP error");
        exit(-1);
    }
    cout << "Server H is up and running using UDP on port "<< H_PORT << endl;

    // backend server work session
    //
    vector<vector<string>> booklist;
    string line;

    ifstream history;
    history.open("./history.txt");

    if (!history.is_open()) {
        cerr << "Failed to open the input file." << endl;
        return 1;
    }

    while (getline(history, line)) {
        vector<string> row;
        istringstream lineStream(line);
        string cell;

        while (getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }

        booklist.push_back(row);
    }

    for (auto & t : booklist){
        if(t[1].front() == ' ') t[1].erase(0, 1);
//        cout << t[0] << endl << t[1] << endl;
    }


    // start work session
    // setting up UDP socket
    while(TRUE){
//        bool flag = FALSE;
        string bookTest;
        char udpResp[100];

        int recvM = recvfrom(udpSkt, udpResp, sizeof(udpResp), 0, nullptr, nullptr);
        if (recvM == -1) {
            cerr << "Receiving bookStatus from backend Server error" << endl;
            close(udpSkt);
            exit(-1);
        }

        string userBookcode(udpResp,recvM);
        string admin = userBookcode.substr(0, 1);
        string bookCode = userBookcode.substr(2);

        if(admin == "Y"){
            cout << "Server S received an inventory status request for code: " << bookCode << endl;
        }
        else{
            cout << "Server S received " << bookCode << " code from the Main Server." << endl;
        }

        // query the book list
        if(admin == "Y"){
            for (auto & t : booklist){
                if (t[0] == bookCode && stoi(t[1]) > 0){
                    bookTest.append("A");
                    bookTest.append("+");
                    bookTest.append(t[1]);
                    break;
                }
                else if(t[0] == bookCode && stoi(t[1]) == 0){
                    bookTest = "D";
                    bookTest.append("+");
                    bookTest.append(t[1]);
                    break;
                }
                else {
                    bookTest = "N";
                }
            }
        }

        else{
            for (auto & t : booklist){
                if (t[0] == bookCode && stoi(t[1]) > 0){
                    bookTest = "A";
                    int result = stoi(t[1]) - 1;
                    t[1] = to_string(result);
                    break;
                }
                else if(t[0] == bookCode && stoi(t[1]) == 0){
                    bookTest = "D";
                    break;
                }
                else {
                    bookTest = "N";
                }
            }
        }

        if(sendto(udpSkt, bookTest.c_str(), bookTest.length(), 0, (sockaddr*)&udp_AddrM, sizeof(udp_AddrM)) == -1){
            cerr << "Sending bookCode to backend ServerH error" << endl;
            close(udpSkt);
            exit(-1);
        }

        if(admin == "Y"){
            cout << "Server H finished sending the inventory status to the Main server using UDP on port " << H_PORT << endl;
        }
        else{
            cout << "Server H finished sending the availability status of code " << bookCode << " to the Main Server using UDP on port " << H_PORT << "." << endl;
        }
    }

    close(udpSkt);
    return 0;
}