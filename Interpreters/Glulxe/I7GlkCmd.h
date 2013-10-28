#include <deque>

struct FrontEndCmd
{
  FrontEndCmd();
  void free(void);
  void read(void);

  int cmd;
  int len;
  void* data;
};
extern std::deque<FrontEndCmd> commands;

void sendCommand(int command, int dataLength, const void* data);
bool readCommand(void);
void readReturnData(void* data, int length);
