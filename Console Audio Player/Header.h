#pragma once
#include <string>
#include <filesystem>


std::string Extensions[] = { ".wav", ".mp3", ".mp4" };
std::wstring wExtensions[] = { L".wav", L".mp3", L".mp4" };

bool hasEnding(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool hasEnding(std::wstring const& fullString, std::wstring const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool isAudio(std::string path) {
    for (int i = 0; i < sizeof(Extensions) / sizeof(Extensions[0]); i++) {
        if (hasEnding(path, Extensions[i])) {
            return true;
        }
    }
    return false;
}

bool isAudio(std::wstring path) {
    for (int i = 0; i < sizeof(Extensions) / sizeof(Extensions[0]); i++) {
        if (hasEnding(path, wExtensions[i])) {
            return true;
        }
    }
    return false;
}

bool isFile(std::string path) {
    for (int i = path.length() - 1; i >= 0; i--) {
        if (path[i] == '.') {
            return true;
        } else if (path[i] == '\\' || path[i] == '/') {
            return false;
        }
    }
    return false;
}

bool isFile(std::wstring path) {
    for (int i = path.length() - 1; i >= 0; i--) {
        if (path[i] == '.') {
            return true;
        } else if (path[i] == '\\' || path[i] == '/') {
            return false;
        }
    }
    return false;
}

bool invalidChar(char c) {
    return !(c >= 0 && c < 128);
}
void stripUnicode(std::string& str) {
    str.erase(remove_if(str.begin(), str.end(), invalidChar), str.end());
}







bool hasEnding(std::filesystem::path const& fullString, std::filesystem::path const& ending) {
    if (fullString.generic_wstring().length() >= ending.generic_wstring().length()) {
        return (0 == fullString.generic_wstring().compare(fullString.generic_wstring().length() - ending.generic_wstring().length(), ending.generic_wstring().length(), ending));
    } else {
        return false;
    }
}

bool isAudio(std::filesystem::path path) {
    for (int i = 0; i < sizeof(Extensions) / sizeof(Extensions[0]); i++) {
        if (hasEnding(path, Extensions[i])) {
            return true;
        }
    }
    return false;
}

bool isFile(std::filesystem::path path) {
    for (int i = path.wstring().length() - 1; i >= 0; i--) {
        if (path.wstring()[i] == '.') {
            return true;
        } else if (path.wstring()[i] == '\\' || path.wstring()[i] == '/') {
            return false;
        }
    }
    return false;
}

std::string tostringnoUTF(std::filesystem::path path) {
    std::wstring temp = path.wstring();
    std::string out;
    for (int i = 0; i < temp.length(); i++) {
        if (static_cast<unsigned char>(temp[i]) > 127) {
            out.push_back('_');
        } else {
            out.push_back(temp[i]);
        }
    }

    return out;
}

namespace math {
    float clamp(float val, float min, float max) {
        return (((val > min) && (val < max)) ? val : (val < min) ? min : max);
    }
}