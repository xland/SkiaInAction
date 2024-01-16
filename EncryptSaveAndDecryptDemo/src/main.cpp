#include <iostream>
#include <windows.h>
#include <wincrypt.h>
#include <wincred.h>
#include <credentialprovider.h>
#include <vector>
#include <string>
#include <random>


bool Encrypt(const std::wstring& plaintext, std::vector<BYTE>& encryptedData)
{
    DATA_BLOB inData, outData;
    inData.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(plaintext.c_str()));
    inData.cbData = (plaintext.size() + 1) * sizeof(wchar_t);
    if (!CryptProtectData(&inData, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &outData))
    {
        DWORD dwError = GetLastError();
        return false; 
    }
    encryptedData.resize(outData.cbData);
    memcpy(encryptedData.data(), outData.pbData, outData.cbData);
    LocalFree(outData.pbData);
    return true;
}
std::wstring Decrypt(const std::vector<BYTE>& encryptedData)
{
    DATA_BLOB inData, outData;
    inData.pbData = const_cast<BYTE*>(encryptedData.data());
    inData.cbData = static_cast<DWORD>(encryptedData.size());
    outData.pbData = NULL;
    outData.cbData = 0;
    if (!CryptUnprotectData(&inData, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &outData))
    {
        DWORD dwError = GetLastError();
        throw std::runtime_error("Failed to decrypt data.");
    }
    std::wstring plaintext(reinterpret_cast<wchar_t*>(outData.pbData), outData.cbData / sizeof(wchar_t) - 1);
    LocalFree(outData.pbData);
    return plaintext;
}

// 定义可打印字符集合（ASCII 32到126）
const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+{}|:<>?~-=[]\\;',./`";
std::string RndStr(size_t length) {
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    std::string str;
    str.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        str += CHARACTERS[distribution(generator)];
    }
    return str;
}

std::wstring appName = L"HikLink";
bool Store(std::string& target)
{
    CREDENTIAL cred = { 0 };
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = appName.data();
    cred.CredentialBlobSize = target.size();
    cred.CredentialBlob = (LPBYTE)(target.data());
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    return CredWrite(&cred, 0) == TRUE;
}

bool Remove() {
    return CredDelete(appName.data(), CRED_TYPE_GENERIC, 0) == TRUE;
}


std::string Read()
{
    CREDENTIAL* cred;
    if (CredRead(appName.data(), CRED_TYPE_GENERIC, 0, &cred) == FALSE) {
        return "";
    }
    std::string result(reinterpret_cast<char*>(cred->CredentialBlob),cred->CredentialBlobSize);
    CredFree(cred);
    return result;
}


/// <summary>
/// 我们在Windows下用DPAPI加密一个随机字符串，然后把这个密文存成文件，放到AppData目录下
/// 用的时候再用DPAPI把这个密文解密出来，用于打开数据库。
/// 
/// 1.  A电脑加密的密文，B电脑无法解密
/// 2.  解密代码是C++写的，解密后打开数据库的JS代码被编译成字节码，无法调试
/// 
/// 如果用户的电脑被入侵了，黑客在用户的电脑上，以用户的身份，执行了解密程序那么是可以得到用户的数据库密码的
/// 这个我们就不管了
/// </summary>
/// <returns></returns>
int main()
{

    //这个随机字符串应该是JS字节码传过来的，这样它就可以在JS字节码里创建一个密文传过来，
    //这里存储的是密文，这样其他应用拿到这个密文之后，不知道解密方式也用不了
    auto str = RndStr(16);
    std::cout << "Rnd Str:" << str << std::endl;

    ////没实际意义
    //std::vector<BYTE> encryptedBytes;
    //Encrypt(str, encryptedBytes);

    ////下面两行代码没实际意义
    //std::string encryptedStr(reinterpret_cast<char*>(encryptedBytes.data()), encryptedBytes.size());
    //std::cout << "Encrypt Str:" << encryptedStr << std::endl;

    ////没实际意义
    //std::wstring decryptedStr = Decrypt(encryptedBytes);
    //std::wcout << L"Decrypted str: " << decryptedStr << std::endl;
    auto flag = Remove();
    std::cout << "Remove Ok" << flag << std::endl;
    flag = Store(str);
    std::cout << "Store Ok" << flag << std::endl;
    auto result = Read();
    std::cout << "Read Ok:" << result << std::endl;



    std::cout << "Hello World!\n";
}