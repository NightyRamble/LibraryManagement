

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define S_PORT 51884
#define L_PORT 52884
#define H_PORT 53884
#define M_UDP_PORT 54884
#define M_CLIENT_TCP_PORT 55884
#define TRUE 1
#define FALSE 0

using namespace std;

struct sockaddr_in udp_AddrS, udp_AddrL, udp_AddrH, udp_AddrM, tcp_serverAddr;

string admin = "N";
string& trim(string& s);

int main(){
    // server boot up session

    //setting up UDP to Science, Literature, History.
    int udpSktM = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSktM == -1){
        perror("Creating UDP error");
        exit(-1);
    }

    //udp sockets configuration
    udp_AddrS.sin_family = AF_INET;
    udp_AddrS.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_AddrS.sin_port = htons(S_PORT);

    udp_AddrL.sin_family = AF_INET;
    udp_AddrL.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_AddrL.sin_port = htons(L_PORT);

    udp_AddrH.sin_family = AF_INET;
    udp_AddrH.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_AddrH.sin_port = htons(H_PORT);

    udp_AddrM.sin_family = AF_INET;
    udp_AddrM.sin_addr.s_addr = inet_addr("127.0.0.1");
    udp_AddrM.sin_port = htons(M_UDP_PORT);
    if(bind(udpSktM, (struct sockaddr*)&udp_AddrM, sizeof(udp_AddrM)) == -1){
        perror("Binding UDP error");
        exit(-1);
    }


    //setting up TCP server
    int tcpSkt = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpSkt == -1){
        perror("Creating TCP error");
        exit(-1);
    }

    //tcp sockets configuration
    tcp_serverAddr.sin_family = AF_INET;
    tcp_serverAddr.sin_port = htons(M_CLIENT_TCP_PORT);
    tcp_serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(tcpSkt, (struct sockaddr*)&tcp_serverAddr, sizeof(tcp_serverAddr)) == -1){
        perror("Binding TCP error");
        exit(-1);
    }
    if(listen(tcpSkt, 4) == -1){
        perror("Listening TCP error");
        exit(-1);
    }

    cout <<"Main Server is up and running." << endl;

    //load member list
    vector<vector<string> > data;
    string line;

    // modify the location!
    ifstream member;
    member.open("./member.txt");

    if (!member.is_open()) {
        cerr << "Failed to open the input file." << endl;
        return 1;
    }

    while (getline(member, line)) {
        vector<string> row;
        istringstream lineStream(line);
        string cell;

        while (getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
    }

    // eliminate the leading ' ' of password
    for (auto & t : data){
        if(t[1].front() == ' ') t[1].erase(0, 1);
//        cout << t[0] << " " << t[1] << endl;
        trim(t[1]);
    }

    cout << "Main Server loaded the member list." << endl;

    // server work session
    // check authentication
    char buffer[4096];
    string username;
    string password;
    int stoClient = accept(tcpSkt, NULL, NULL);
    if (stoClient == -1) {
        cerr << "Accept failed." << endl;
        close(stoClient);
        return 1;
    }

    while(TRUE){

        int recvlogin = recv(stoClient, buffer, sizeof(buffer), 0);
        buffer[recvlogin] = '\0';

        if (recvlogin <= 0) {
            cerr << "Failed to receive data from the client." << endl;
            close(stoClient);
            return 1;
        }
        cout << "Main Server received the username and password from the client using TCP over port "
                << M_CLIENT_TCP_PORT << "." << endl;


        // slice the username + password
        string loginTest(buffer, recvlogin);
//        cout << loginTest << endl;
        size_t plus = loginTest.find("+");
        username = loginTest.substr(0, plus);
        password = loginTest.substr(plus+1);
//        cout << "the received username is: " << username << endl;
//        cout << "the received password is: " << password << endl;


        bool isLogin = FALSE;


        //compare with database
        for (auto & d : data){
            if(d[0] == username){
//                cout << "username " << d[0] << endl;
//                cout << "password " << d[1].length() << endl;
//                cout << "received password is: " << password.length() << endl;
//                int nani = d[1] == password ? 1 :0;
//                cout << nani << endl;
                if(d[1] == password){
//                    cout << "the password is: " << d[1] << endl;
                    // all match
                    isLogin = TRUE;
                    loginTest = "A";
                    cout << "Password " << password << " matches the username. Send a reply to the client." << endl;
                    if (username == "firns") admin = "Y";
                    break;
//                    send(stoClient, loginTest.c_str(), loginTest.length(), 0);
//                    flag[0] = TRUE;
//                    flag[1] = FALSE;
//                    flag[2] = FALSE;
//                    break;
                }
                else{

                    //password doesnt match
                    loginTest = "B";
                    admin = "N";
//                    send(stoClient, loginTest.c_str(), loginTest.length(), 0);
//                    break;
//                    flag[1] = TRUE;
                }
            }
            if(!isLogin && loginTest != "B") {
                loginTest = "C";
//                send(stoClient, loginTest.c_str(), loginTest.length(), 0);
//                break;
            }

        }
        if(loginTest == "B") cout << "Password " << password <<" does not match the username. Send a reply to the client." << endl;
        if(loginTest == "C") cout << username << " is not registered. Send a reply to the client." << endl;

        send(stoClient, loginTest.c_str(), loginTest.length(), 0);

        // if login success, start work session
        // here to insert extra admin authorization
        // first compare username, if NF continue
        if (isLogin) break;
//        if (flag[2]){
//            cout << username << " is not registered. Send a reply to the client." << endl;
////            cout << loginTest << endl;
//            send(stoClient, loginTest.c_str(), loginTest.length(), 0);
//            continue;
//        }
//        if (flag[1]){
//            cout << "Password " << password <<" does not match the username. Send a reply to the client." << endl;
////            cout << loginTest << endl;
//            send(stoClient, loginTest.c_str(), loginTest.length(), 0);
//            continue;
//        }
    }

    // start work session
    // setting up UDP socket

    while(TRUE) {
        char bookStatus[100];   // for bookcode uploaded by client
        char udpResp[100];  // for query result from SLH

        int recvBook = recv(stoClient, bookStatus, sizeof(bookStatus), 0);
        bookStatus[recvBook] = '\0';

        if (recvBook <= 0) {
            cerr << "Failed to receive data from the client." << endl;
            close(stoClient);
            return 1;
        }

        cout << "Main Server received the book request from client using TCP over port " << M_CLIENT_TCP_PORT << endl;

        string bookCode(bookStatus, recvBook);
        string type = bookCode.substr(0, 1);
        string userBookcode =  admin + "+" + bookCode;
        string udpport;
//        cout << userBookcode << endl;

//        cout << type << endl;

        // query the backend server corresponding to the book type
        if (type == "S"){
            udpport = to_string(S_PORT);
            if (sendto(udpSktM, userBookcode.c_str(), userBookcode.length(), 0, (sockaddr*)&udp_AddrS, sizeof(udp_AddrS)) == -1){
                cerr << "Sending bookCode to backend ServerS error" << endl;
                close(udpSktM);
                exit(-1);
            }
            cout << "Found " << bookCode << " located at Server " << type << ". Send to Server " << type << "." << endl;

        }
        else if (type == "L"){
            udpport = to_string(L_PORT);
            if(sendto(udpSktM, userBookcode.c_str(), userBookcode.length(), 0, (sockaddr*)&udp_AddrL, sizeof(udp_AddrL)) == -1){
                cerr << "Sending bookCode to backend ServerL error" << endl;
                close(udpSktM);
                exit(-1);
            }
            cout << "Found " << bookCode << " located at Server " << type << ". Send to Server " << type << "." << endl;

        }
        else if (type == "H"){
            udpport = to_string(H_PORT);
            if(sendto(udpSktM, userBookcode.c_str(), userBookcode.length(), 0, (sockaddr*)&udp_AddrH, sizeof(udp_AddrH)) == -1){
                cerr << "Sending bookCode to backend ServerH error" << endl;
                close(udpSktM);
                exit(-1);
            }
            cout << "Found " << bookCode << " located at Server " << type << ". Send to Server " << type << "." << endl;

        }
        // if bookCode not starting with SLH
        else {
            cout << "Did not find " << bookCode << " in the book code list." << endl;
            string replytoClient = "Not able to find the book-code " + bookCode + " in the system.";
            send(stoClient, replytoClient.c_str(), replytoClient.length(), 0);
            continue;
        }

        int recvSLH = recvfrom(udpSktM, udpResp, sizeof(udpResp), 0, nullptr, nullptr);
        if (recvSLH == -1) {
            cerr << "Receiving bookStatus from backend Server error" << endl;
            close(udpSktM);
            exit(-1);
        }

        cout <<"Main Server received from server " << type << " the book status result using UDP over port " << udpport << endl;

        string status = "N";
        string value = "-1";
        string bookResp(udpResp, recvSLH);
        if(admin == "Y" && bookResp != "N"){
            status = bookResp.substr(0,1);
            value = bookResp.substr(2);
        }
        else{
            status = bookResp;
        }

        // check here if non-admin query return value = null

//        cout << bookResp << endl;
        if (status == "A"){
            if(admin == "Y") cout << "Number of books " << bookCode << " available is: " << value << "." << endl;
            else cout << "The requested book " + bookCode + " is available in the library." << endl;
        }
        else if (status == "D"){
            if(admin == "Y") cout << "Number of books " << bookCode << " available is: " << 0 << "." << endl;
            cout << "The requested book " + bookCode + " is NOT available in the library." << endl;
        }
        else {
            cout << "Not able to find the " + bookCode + " in the system." << endl;
        }

        string replytoClient;
        send(stoClient, bookResp.c_str(), bookResp.length(), 0);

        cout << "Main Server sent the book status to the client." << endl;
    }

    close(udpSktM);
    close(tcpSkt);

}


string& trim(string& s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
    return s;
}