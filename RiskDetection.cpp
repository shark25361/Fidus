#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#ifndef MAX_DATA_LENGTH
#include "SerialPort.hpp"
#endif

#define SEND
#define DATA_LENGTH 255

SerialPort *arduino;
char *arduinoPort;
char returnData[DATA_LENGTH];

void interact(char *arduinoPort);
void connectWithArduino(void);

int main(int argc,char *argv[])
{
    std::cout << "Enter the COM port name that the Arduino is connected to: " << std::endl;
    std::cin >> arduinoPort;
    interact(arduinoPort);
}

void interact(char *arduinoPort)
{
    arduino = new SerialPort(arduinoPort);
    connectWithArduino();
    char action[1];
    char temp;
    char *SSID;
    char *passwd;
    char *localIP;

    std::cout << "This program can be used to interact with the arduino via USB in a user friendly manner." << std::endl;
    std::cout << "Press 'r' to restart the arduino." << std::endl;
    std::cout << "Press 't' to get current roll and pitch." << std::endl;
    std::cout << "Press 'w' to enter or update wifi credentials." << std::endl;
    
    while(arduino->isConnected())
    {
        std::cin >> action[0];
        switch(action[0])
        {
            case 'r':
                if(!arduino->writeSerialPort(action,sizeof(action)))
                {
                    std::cout << "Error writing to arduino serial port." << std::endl;
                    return;
                }
                break;
            case 't':
                char roll[32],pitch[32];
                arduino->readSerialPort(roll,sizeof(roll));
                arduino->readSerialPort(pitch,sizeof(pitch));
                break;
            case 'w':
                std::cout << "Please enter wifi SSID and password, space separated." << std::endl;
                if(!arduino->writeSerialPort(action,sizeof(action)))
                {
                    std::cout << "Error writing to arduino serial port." << std::endl;
                    return;
                }
                std::cin >> SSID >> passwd;
                if(!arduino->writeSerialPort(SSID,sizeof(SSID)))
                {
                    std::cout << "Error writing to arduino serial port." << std::endl;
                    return;
                }
                Sleep(20);
                if(!arduino->writeSerialPort(passwd,sizeof(passwd)))
                {
                    std::cout << "Error writing to arduino serial port." << std::endl;
                    return;
                }
                Sleep(50);
                arduino->readSerialPort(localIP,64);
                std::cout << "Local IP of arduino is ";
                for(int i = 0;i < sizeof(localIP);i++)
                {
                    std::cout << localIP[i];
                }
                std::cout << "." << std::endl;
                break;
            default:
               std::cout << "Invalid input." << std::endl;
                break;
        } 
    }
}

void connectWithArduino(void)
{
    //better than recursion
    //avoid stack overflows
    while(1)
    {
        // ui - searching
        std::cout << "Searching in progress";
        std::cout << "If the arduino is not found within 1 minute, please recheck the port.";
        // wait connection
        while (!arduino->isConnected())
        {
            Sleep(100);
            std::cout << ".";
            arduino = new SerialPort(arduinoPort);
        }

        //Checking if arduino is connected or not
        if (arduino->isConnected())
        {
            std::cout  << std::endl << "Connection established at port " << arduinoPort  << "." << std::endl;
        }
    }
}
