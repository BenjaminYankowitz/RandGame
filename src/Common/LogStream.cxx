export module Common:LogStream;
import std;

class IgnoreStreamBuf : public std::streambuf {
public:
  IgnoreStreamBuf() = default;

protected:
  std::streamsize xsputn(const char_type * /*s*/, std::streamsize count) override {
    return count;
  }
};

export namespace Logging {

IgnoreStreamBuf ignoreBuf; //NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::ostream log(std::cerr.rdbuf()); //NOLINT(cppcoreguidelines-avoid-non-const-global-variables) //NOLINT(cppcoreguidelines-interfaces-global-init)

} // namespace Logging