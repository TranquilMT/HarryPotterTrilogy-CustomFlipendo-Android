#pragma once
#include <filesystem>
#include <system_error>

// Retail configurations can list developer-only folders. Directory access is
// fallible, including when a folder disappears while it is being enumerated.
// File processing errors still propagate to the caller instead of being hidden.
template<class Visitor>
std::error_code VisitDataFiles(const std::filesystem::path& folder, Visitor&& visitor)
{
    std::error_code error;
    std::filesystem::directory_iterator current(folder, error), end;
    if (error) return error;
    while (current != end)
    {
        std::error_code statusError;
        if (current->is_regular_file(statusError) && !statusError)
            visitor(*current);
        current.increment(error);
        if (error) return error;
    }
    return {};
}
