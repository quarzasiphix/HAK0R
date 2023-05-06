#include "proc.hpp"

uintptr_t proc::GetAddressFromSignature(std::vector<int> signature) {
    if (this->address == NULL || this->hProcess == NULL) {
        return NULL;
    }
    std::vector<byte> memBuffer(this->pSize);
    if (!ReadProcessMemory(this->hProcess, (LPCVOID)(this->hProcess), memBuffer.data(), this->pSize, NULL)) {
        std::cout << GetLastError() << std::endl;
        return NULL;
    }
    for (int i = 0; i < this->pSize; i++) {
        for (uintptr_t j = 0; j < signature.size(); j++) {
            if (signature.at(j) != -1 && signature[j] != memBuffer[i + j])
                //std::cout << std::hex << signature.at(j) << std::hex << memBuffer[i + j] << std::endl;
                break;
            if (signature[j] == memBuffer[i + j] && j > 0)
                std::cout << std::hex << int(signature[j]) << std::hex << int(memBuffer[i + j]) << j << std::endl;
            if (j + 1 == signature.size())
                return static_cast<uintptr_t>(this->pBaseAddress) + i;
        }
    }
    return NULL;
}
