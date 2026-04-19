#include "modules/network/NetworkAuditModule.h"

#include "platform/windows/Encoding.h"
#include "platform/windows/WinError.h"

#include <winsock2.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Ws2tcpip.h>

#include <set>
#include <string>
#include <vector>

namespace {

std::string sockaddrToString(const SOCKADDR* address) {
    if (address == nullptr) {
        return "неизвестно";
    }

    wchar_t buffer[INET6_ADDRSTRLEN] {};
    if (address->sa_family == AF_INET) {
        const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(address);
        if (InetNtopW(AF_INET, const_cast<IN_ADDR*>(&ipv4->sin_addr), buffer, INET_ADDRSTRLEN) != nullptr) {
            return sec::toUtf8(buffer);
        }
    } else if (address->sa_family == AF_INET6) {
        const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(address);
        if (InetNtopW(AF_INET6, const_cast<IN6_ADDR*>(&ipv6->sin6_addr), buffer, INET6_ADDRSTRLEN) != nullptr) {
            return sec::toUtf8(buffer);
        }
    }

    return "неизвестно";
}

std::string queryProcessName(const DWORD pid) {
    if (pid == 0) {
        return "System";
    }

    const auto process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process == nullptr) {
        return "PID " + std::to_string(pid);
    }

    DWORD size = MAX_PATH;
    std::wstring path(size, L'\0');
    std::string result = "PID " + std::to_string(pid);
    if (QueryFullProcessImageNameW(process, 0, path.data(), &size)) {
        path.resize(size);
        const auto separator = path.find_last_of(L"\\/");
        const auto executable = separator == std::wstring::npos ? path : path.substr(separator + 1);
        result = sec::toUtf8(executable) + " (PID " + std::to_string(pid) + ")";
    }
    CloseHandle(process);
    return result;
}

}  // namespace

namespace sec {

std::vector<AuditItem> NetworkAuditModule::collect() const {
    std::vector<AuditItem> items;

    AuditItem adapters;
    adapters.category = "Сеть";
    adapters.title = "Активные сетевые адаптеры";
    adapters.status = AuditStatus::info;

    ULONG bufferSize = 0;
    GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &bufferSize);
    std::vector<BYTE> buffer(bufferSize);
    auto* addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buffer.data());

    const auto adapterStatus = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, addresses, &bufferSize);
    if (adapterStatus == NO_ERROR) {
        bool hasOperationalAdapter = false;
        for (auto* adapter = addresses; adapter != nullptr; adapter = adapter->Next) {
            if (adapter->OperStatus != IfOperStatusUp) {
                continue;
            }

            hasOperationalAdapter = true;
            std::string line = toUtf8(adapter->FriendlyName);

            std::vector<std::string> ips;
            for (auto* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next) {
                ips.push_back(sockaddrToString(unicast->Address.lpSockaddr));
            }
            if (!ips.empty()) {
                line += " :: ";
                for (std::size_t index = 0; index < ips.size(); ++index) {
                    if (index != 0) {
                        line += ", ";
                    }
                    line += ips[index];
                }
            }

            adapters.details.push_back(line);
        }

        adapters.status = hasOperationalAdapter ? AuditStatus::ok : AuditStatus::warning;
        adapters.recommendation = hasOperationalAdapter
            ? "Проверь, что активны только ожидаемые сетевые адаптеры."
            : "Активные адаптеры не найдены. Проверь драйверы и текущее сетевое подключение.";
    } else {
        adapters.status = AuditStatus::error;
        adapters.details = {"Не удалось получить список адаптеров: " + formatWindowsErrorMessage(adapterStatus)};
        adapters.recommendation = "Проверь доступность Windows IP Helper API.";
    }
    items.push_back(std::move(adapters));

    AuditItem listeners;
    listeners.category = "Сеть";
    listeners.title = "Слушающие TCP-порты";
    listeners.status = AuditStatus::info;

    ULONG tcpBufferSize = 0;
    GetExtendedTcpTable(nullptr, &tcpBufferSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0);
    std::vector<BYTE> tcpBuffer(tcpBufferSize);
    auto* table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID*>(tcpBuffer.data());
    const auto tableStatus = GetExtendedTcpTable(table, &tcpBufferSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0);

    if (tableStatus == NO_ERROR) {
        std::set<int> sensitivePorts {139, 445, 3389, 5985, 5986};
        bool sensitivePortFound = false;

        for (DWORD index = 0; index < table->dwNumEntries; ++index) {
            const auto& row = table->table[index];
            const auto port = ntohs(static_cast<u_short>(row.dwLocalPort));
            listeners.details.push_back("Порт " + std::to_string(port) + " :: " + queryProcessName(row.dwOwningPid));
            if (sensitivePorts.contains(port)) {
                sensitivePortFound = true;
            }
        }

        listeners.status = sensitivePortFound ? AuditStatus::warning : AuditStatus::ok;
        listeners.recommendation = sensitivePortFound
            ? "На хосте слушают чувствительные порты управления или шаринга. Проверь, действительно ли они нужны."
            : "Среди слушающих портов не найдено типовых высокорисковых портов управления.";
    } else {
        listeners.status = AuditStatus::error;
        listeners.details = {"Не удалось получить список TCP-listeners: " + formatWindowsErrorMessage(tableStatus)};
        listeners.recommendation = "Проверь права доступа и доступность IP Helper API.";
    }
    items.push_back(std::move(listeners));

    return items;
}

}  // namespace sec
