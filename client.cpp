
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>



#define M_UDP_PORT 44884
#define M_CLIENT_TCP_PORT 55884
#define offset 5
#define TRUE 1

using namespace std;
struct sockaddr_in tcp_clientAddr;

string encrypt(const string& input) {
    string result = input;
    char ofst = offset;

    for (char& c : result){
        if(isdigit(c)){
            c = (c - '0' + ofst) % 10 + '0';
        }
        else if(isalpha(c)){
            char base = islower(c) ? 'a' : 'A';
            c = (c - base + ofst) % 26 + base;
        }
    }
    return result;
}

int main(){

    // client initiate session

    int tcpSkt = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpSkt == -1){
        perror("Creating TCP error");
        close(tcpSkt);
        exit(-1);
    }

    //tcp sockets configuration
    tcp_clientAddr.sin_family = AF_INET;
    tcp_clientAddr.sin_port = htons(M_CLIENT_TCP_PORT);
    tcp_clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(tcpSkt, (struct sockaddr*)&tcp_clientAddr, sizeof(tcp_clientAddr)) == -1) {
        perror("Server connection error");
        close(tcpSkt);
        return 1;
    }

    //test client dynamic port assignment
    sockaddr_in portAddr;
    int clientPort;
    socklen_t addrlen = sizeof(portAddr);
    if (getsockname(tcpSkt, (struct sockaddr*)&portAddr, &addrlen) == -1){
        perror("getsockname");
    }
    else {
        clientPort = ntohs(portAddr.sin_port);
//        cout << "TCP client port: " << port << endl;
    }

    cout << "Client is up and running." << endl;

    // client work session
    // Login and confirmation
    string username;
    string password;
    char response[10];
    string u;
    string admin;

    //check again code below
    //test here!
    while (TRUE){
        cout << "Please enter the username:";
        getline(cin, username);
        cout << "Please enter the password:";
        getline(cin, password);

        u = username;
        string p = password;
        username = encrypt(username);
        password = encrypt(password);

        //send to server
        string login = username + "+" + password;

        if (send(tcpSkt, login.c_str(), login.length(), 0) == -1) {
            cerr << "Error in sending username and password." << endl;
            close(tcpSkt);
            exit(-1);
        }

        cout << u << " sent an authentication request to the Main Server." << endl;

        int loginTest = recv(tcpSkt, response, sizeof(response), 0);
        response[loginTest] = '\0'; //Null terminate
//        cout << response << endl;

        if(string(response) == "A"){
            // login successful
            cout << u << " received the result of authentication from Main Server using TCP over port "
                 << clientPort << ". Authentication is successful." << endl;
            if(username == "firns"){
                admin = "Y";
            }
            break;
        }

        if(string(response) == "B"){
            // password doesn't match
            cout << u << " received the result of authentication from Main Server using TCP over port "
                 << clientPort << ". Authentication failed: Password does not match." << endl;

        }

        if(string(response) == "C"){
            // username not found
            cout << u << " received the result of authentication from Main Server using TCP over port "
                 << clientPort << ". Authentication failed: Username not found." << endl;

        }
        else if (loginTest <= 0){
            cerr << "Error in receiving response from server." << endl;
        }

        // if login failed, continue
    }


    // start work session
    string bookCode;
    char bookStatus[100];

    while(TRUE){
        cout << "Please enter the book code to query:";
        getline(cin, bookCode);

        if(send(tcpSkt, bookCode.c_str(), bookCode.length(), 0) == -1){
            cerr << "Sending bookCode error" << endl;
            close(tcpSkt);
            exit(-1);
        }

        if(admin == "Y"){
            cout <<"Request sent to the Main Server with Admin rights." << endl;
        }
        else {
            cout << u << " sent the request to the Main Server." << endl;
        }

        int recvBook = recv(tcpSkt, bookStatus, sizeof(bookStatus), 0);
        if (recvBook <= 0) {
            cerr << "Failed to receive book status from the server." << endl;
            close(tcpSkt);
            return 1;
        }

        string status = "N";
        string value = "-1";
        string bookResp(bookStatus, recvBook);
        if(admin == "Y" && bookResp != "N"){
            status = bookResp.substr(0,1);
            value = bookResp.substr(2);
        }
        else{
            status = bookResp;
        }
//        cout << bookResp << endl;


        cout << "Response received from the Main Server on TCP port: " << clientPort << endl;

        if (status == "A"){
            if(admin == "Y"){
                cout << "Total number of book " << bookCode << " available = " << value << endl;
            }
            else{
                cout << "The requested book " + bookCode + " is available in the library." << endl;
            }

        }
        else if (status == "D"){
            if(admin == "Y"){
                cout << "Total number of book " << bookCode << " available = " << 0 << endl;
            }
            else{
                cout << "The requested book " + bookCode + " is NOT available in the library." << endl;
            }
        }
        else {
            cout << "Not able to find the " + bookCode + " in the system." << endl;
        }


    }

    close(tcpSkt);
}