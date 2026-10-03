#define KJ_STD_COMPAT

#include "capnproto.hpp"
#include "centipede/util/return_types.hpp"
#include "entry.capnp.h"
#include <capnp/serialize-packed.h>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <fstream>
#include <ios>
#include <ranges>

namespace centipede::writer
{
    auto Capnproto::init() -> VoidError
    {
        output_file_.open(config_.out_filename, std::ios::out | std::ios::binary);
        if (not output_file_.is_open())
        {
            return ErrorCode::Error( std::format("Cannot open the file {:?}", config_.out_filename) );
        }

        return {};
    }

    auto Capnproto::write_current_entry() -> ResultError<std::size_t>
    {
        auto entry = message_.initRoot<capnproto::Entry>();

        auto entrypoints = entry.initEntrypoint(static_cast<uint32_t>(entrypoints_.size()));

        for (auto [entrypoint_out, entrypoint_in] : std::views::zip(std::views::all(entrypoints), entrypoints_))
        {
            const auto& locals = entrypoint_in.get_locals();
            auto local_derivs = entrypoint_out.initLocalDerivs(static_cast<uint32_t>(locals.size()));
            for (auto [idx, local_out, local_in] : std::views::zip(std::views::iota(0U), local_derivs, locals))
            {
                local_out.setId(idx);
                local_out.setValue(local_in.value);
                local_out.setError(local_in.error);
            }

            const auto& globals = entrypoint_in.get_globals();
            auto global_derivs = entrypoint_out.initGlobalDerivs(static_cast<uint32_t>(globals.size()));

            for (auto [global_out, global_in] : std::views::zip(global_derivs, globals))
            {
                const auto& [idx, global_val] = global_in;
                global_out.setId(idx);
                global_out.setValue(global_val.value);
                global_out.setError(global_val.error);
            }
        }
        capnp::writePackedMessageToFd(output_file_.native_handle(), message_);

        entrypoints_.clear();
        return 0;
    }
} // namespace centipede::writer
