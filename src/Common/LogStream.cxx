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

IgnoreStreamBuf ignoreBuf;

std::ostream log(std::cerr.rdbuf());

} // namespace Logging