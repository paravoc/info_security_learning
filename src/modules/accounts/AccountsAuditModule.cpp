#include "modules/accounts/AccountsAuditModule.h"

#include "core/StringUtils.h"
#include "platform/windows/Encoding.h"
#include "platform/windows/WinError.h"

#include <Windows.h>
#include <Lm.h>

#include <memory>
#include <string>
#include <vector>

namespace {

struct NetApiBufferDeleter {
    void operator()(void* pointer) const {
        if (pointer != nullptr) {
            NetApiBufferFree(pointer);
        }
    }
};

std::string flagsToDescription(const DWORD flags) {
    std::vector<std::string> parts;
    if ((flags & UF_ACCOUNTDISABLE) != 0) {
        parts.emplace_back("отключена");
    }
    if ((flags & UF_LOCKOUT) != 0) {
        parts.emplace_back("заблокирована");
    }
    if ((flags & UF_PASSWD_NOTREQD) != 0) {
        parts.emplace_back("пароль не обязателен");
    }
    if ((flags & UF_DONT_EXPIRE_PASSWD) != 0) {
        parts.emplace_back("пароль не истекает");
    }
    if (parts.empty()) {
        parts.emplace_back("стандартная политика");
    }

    std::string description;
    for (std::size_t index = 0; index < parts.size(); ++index) {
        if (index != 0) {
            description += ", ";
        }
        description += parts[index];
    }
    return description;
}

std::wstring resolveAdministratorsGroupName() {
    std::vector<BYTE> sidBuffer(SECURITY_MAX_SID_SIZE);
    auto* sid = reinterpret_cast<PSID>(sidBuffer.data());
    DWORD sidSize = static_cast<DWORD>(sidBuffer.size());
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, sid, &sidSize)) {
        return L"Administrators";
    }

    DWORD nameLength = 0;
    DWORD domainLength = 0;
    SID_NAME_USE use {};
    LookupAccountSidW(nullptr, sid, nullptr, &nameLength, nullptr, &domainLength, &use);

    std::wstring name(nameLength, L'\0');
    std::wstring domain(domainLength, L'\0');
    if (!LookupAccountSidW(nullptr, sid, name.data(), &nameLength, domain.data(), &domainLength, &use)) {
        return L"Administrators";
    }

    name.resize(nameLength);
    return name;
}

std::string userModalsTimeToString(const DWORD value) {
    if (value == TIMEQ_FOREVER || value == MAXDWORD) {
        return "Без ограничения";
    }
    return sec::humanReadableDurationSeconds(value);
}

}  // namespace

namespace sec {

std::vector<AuditItem> AccountsAuditModule::collect() const {
    std::vector<AuditItem> items;

    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    USER_INFO_3* usersRaw = nullptr;
    const auto usersStatus = NetUserEnum(nullptr,
                                         3,
                                         FILTER_NORMAL_ACCOUNT,
                                         reinterpret_cast<LPBYTE*>(&usersRaw),
                                         MAX_PREFERRED_LENGTH,
                                         &entriesRead,
                                         &totalEntries,
                                         &resumeHandle);

    std::unique_ptr<USER_INFO_3, NetApiBufferDeleter> users(usersRaw);
    AuditItem localUsers;
    localUsers.category = "Учетные записи";
    localUsers.title = "Локальные пользователи";
    localUsers.status = AuditStatus::info;

    if (usersStatus == NERR_Success && users != nullptr) {
        bool insecureUserFound = false;
        for (DWORD index = 0; index < entriesRead; ++index) {
            const auto& user = users.get()[index];
            const auto flagsDescription = flagsToDescription(user.usri3_flags);
            localUsers.details.push_back(toUtf8(user.usri3_name) + " :: " + flagsDescription);

            if ((user.usri3_flags & UF_PASSWD_NOTREQD) != 0 || (user.usri3_flags & UF_DONT_EXPIRE_PASSWD) != 0) {
                insecureUserFound = true;
            }
        }

        localUsers.status = insecureUserFound ? AuditStatus::warning : AuditStatus::ok;
        localUsers.recommendation = insecureUserFound
            ? "Требуй пароли и избегай бессрочных паролей для повседневных локальных учеток."
            : "Флаги локальных учетных записей выглядят разумно.";
    } else {
        localUsers.status = AuditStatus::error;
        localUsers.details = {"Не удалось перечислить локальных пользователей: " + formatWindowsErrorMessage(usersStatus)};
        localUsers.recommendation = "Запусти инструмент в обычной Windows-сессии, где доступны локальные account APIs.";
    }
    items.push_back(std::move(localUsers));

    AuditItem admins;
    admins.category = "Учетные записи";
    admins.title = "Состав группы администраторов";
    admins.status = AuditStatus::info;

    const auto adminGroupName = resolveAdministratorsGroupName();
    LOCALGROUP_MEMBERS_INFO_1* membersRaw = nullptr;
    entriesRead = 0;
    totalEntries = 0;
    const auto membersStatus = NetLocalGroupGetMembers(nullptr,
                                                       adminGroupName.c_str(),
                                                       1,
                                                       reinterpret_cast<LPBYTE*>(&membersRaw),
                                                       MAX_PREFERRED_LENGTH,
                                                       &entriesRead,
                                                       &totalEntries,
                                                       nullptr);

    std::unique_ptr<LOCALGROUP_MEMBERS_INFO_1, NetApiBufferDeleter> members(membersRaw);
    if (membersStatus == NERR_Success && members != nullptr) {
        for (DWORD index = 0; index < entriesRead; ++index) {
            admins.details.push_back(toUtf8(members.get()[index].lgrmi1_name));
        }
        admins.status = entriesRead > 2 ? AuditStatus::warning : AuditStatus::ok;
        admins.recommendation = entriesRead > 2
            ? "Проверь, действительно ли всем участникам нужны права локального администратора."
            : "Размер группы администраторов небольшой, ею проще управлять.";
    } else {
        admins.status = AuditStatus::error;
        admins.details = {"Не удалось прочитать группу администраторов: " + formatWindowsErrorMessage(membersStatus)};
        admins.recommendation = "Проверь доступность local group APIs и текущие права.";
    }
    items.push_back(std::move(admins));

    AuditItem policy;
    policy.category = "Учетные записи";
    policy.title = "Парольная политика и блокировка";
    policy.status = AuditStatus::info;

    USER_MODALS_INFO_0* modals0Raw = nullptr;
    USER_MODALS_INFO_3* modals3Raw = nullptr;
    const auto modals0Status = NetUserModalsGet(nullptr, 0, reinterpret_cast<LPBYTE*>(&modals0Raw));
    const auto modals3Status = NetUserModalsGet(nullptr, 3, reinterpret_cast<LPBYTE*>(&modals3Raw));

    std::unique_ptr<USER_MODALS_INFO_0, NetApiBufferDeleter> modals0(modals0Raw);
    std::unique_ptr<USER_MODALS_INFO_3, NetApiBufferDeleter> modals3(modals3Raw);

    if (modals0Status == NERR_Success && modals3Status == NERR_Success && modals0 != nullptr && modals3 != nullptr) {
        policy.details = {
            "Минимальная длина пароля: " + std::to_string(modals0->usrmod0_min_passwd_len),
            "История паролей: " + std::to_string(modals0->usrmod0_password_hist_len),
            "Максимальный возраст пароля: " + userModalsTimeToString(modals0->usrmod0_max_passwd_age),
            "Минимальный возраст пароля: " + userModalsTimeToString(modals0->usrmod0_min_passwd_age),
            "Порог блокировки: " + std::to_string(modals3->usrmod3_lockout_threshold),
            "Длительность блокировки: " + userModalsTimeToString(modals3->usrmod3_lockout_duration),
            "Окно наблюдения: " + userModalsTimeToString(modals3->usrmod3_lockout_observation_window),
        };

        const auto weakMinimumLength = modals0->usrmod0_min_passwd_len < 8;
        const auto missingLockout = modals3->usrmod3_lockout_threshold == 0;
        const auto weakHistory = modals0->usrmod0_password_hist_len < 5;
        const auto neverExpire = modals0->usrmod0_max_passwd_age == TIMEQ_FOREVER || modals0->usrmod0_max_passwd_age == MAXDWORD;

        policy.status = (weakMinimumLength || missingLockout || weakHistory || neverExpire) ? AuditStatus::warning : AuditStatus::ok;
        policy.recommendation = policy.status == AuditStatus::warning
            ? "Ориентируйся минимум на 8-12 символов, историю паролей и ненулевой порог блокировки."
            : "Парольная политика выглядит нормально для рабочей станции.";
    } else {
        policy.status = AuditStatus::error;
        policy.details = {
            "Не удалось получить парольную политику уровня 0: " + formatWindowsErrorMessage(modals0Status),
            "Не удалось получить парольную политику уровня 3: " + formatWindowsErrorMessage(modals3Status),
        };
        policy.recommendation = "Проверь, доступны ли local security policy APIs на этой редакции Windows.";
    }
    items.push_back(std::move(policy));

    return items;
}

}  // namespace sec
