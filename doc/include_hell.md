 [text](include_audit.txt)
maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp
grep -RhoP '^#include\s*[<]\K[^>]+' ./src ./include \
| sort \
| uniq -c > ./doc/include_audit.txt
      1 commdlg.h
      6 cstdio
      2 cwchar
      1 guiddef.h
      1 psapi.h
      1 shellapi.h
      1 stdio.h
      7 string
      1 tlhelp32.h
      1 vector
      1 WebView2.h
      8 windows.h
      2 winsock2.h
      1 wrl/client.h
      2 ws2tcpip.h
7 string --> std::
maxko@MAX_NOTE UCRT64 ~/dev/AppPortCpp
$ grep -rE "std::[a-z_0-9]+\s*\(|\.[a-z_0-9]+\s*\(" src/ include/ --include=*.{cpp,hpp,h} \
| grep -vE "include|if\s*\(|while\s*\(|for\s*\("

Метод / Функция,Пример из кода (grep),Итого раз
.c_str(),"full_path.c_str(), msg.c_str(), ts.c_str()",21
.find(),"json.find(pattern), json.find(L"":"", key_pos)",7
.size(),"(int)str.size(), (DWORD)utf8.size()",3
.resize(),text.resize(readed / sizeof(wchar_t)),3
.back(),woutput.back() == L'\n',2
.pop_back(),woutput.pop_back(),2
.data(),"ReadFile(file, text.data(), ...)",2
std::wstring(),"std::wstring(result.node.ok ? L""true"" : L""false"")",4
.substr(),"json.substr(first_quote + 1, ...)",1
.push_back(),out_result.push_back(item),1

вар2
Метод / Функция,Пример использования,Раз,Файлы (основные)
.c_str(),full_path.c_str(),21,"env_check, logsystem, main, save_config"
.find(),json.find(pattern),7,load_config
std::wstring(),"std::wstring(L""true"")",4,"env_check, logsystem"
.size(),(int)str.size(),3,"env_check, logsystem"
.resize(),text.resize(size),3,"load_config, logsystem"
.data(),"ReadFile(..., text.data())",2,"load_config, logsystem"
.back(),woutput.back(),2,env_check
.pop_back(),woutput.pop_back(),2,env_check
.substr(),json.substr(...),1,load_config
.push_back(),out_result.push_back(),1,env_check

