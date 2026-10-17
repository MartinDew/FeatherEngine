// C++ module wrapper for taywee/args (`import args;`). args.hxx is only ever
// textually included here, never next to `import std;`, which GCC can't merge.
module;

#include <args.hxx>

export module args;

export namespace args {
using args::get;
using args::Glyphs;
using args::Wrap;

#ifdef ARGS_NOEXCEPT
using args::Error;
#else
using args::Completion;
using args::Error;
using args::ExtraError;
using args::Help;
using args::MapError;
using args::ParseError;
using args::RequiredError;
using args::SubparserError;
using args::UsageError;
using args::ValidationError;
#endif

using args::EitherFlag;
using args::HelpParams;
using args::Matcher;
using args::Nargs;
using args::Options;
using args::operator|;
using args::operator&;
using args::operator<<;

using args::Base;
using args::Command;
using args::FlagBase;
using args::GlobalOptions;
using args::Group;
using args::NamedBase;
using args::PositionalBase;
using args::Subparser;
using args::ValueFlagBase;
using args::ValueReader;

using args::ArgumentParser;

using args::ActionFlag;
using args::CompletionFlag;
using args::CounterFlag;
using args::Flag;
using args::HelpFlag;
using args::ImplicitValueFlag;
using args::MapFlag;
using args::MapFlagList;
using args::NargsValueFlag;
using args::ValueFlag;
using args::ValueFlagList;

using args::MapPositional;
using args::MapPositionalList;
using args::Positional;
using args::PositionalList;
} // namespace args
