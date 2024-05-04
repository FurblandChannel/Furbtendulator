#include <vector>
namespace PlugThruDevice {
namespace SuperMagicCard {
extern	std::vector<uint8_t> fileData;
extern	bool	hardReset;
extern	bool	haveFileToSend;

void	doAnyTransfer(void);
void	addToReceiveQueue(uint8_t);

} // namespace SuperMagicCard
} // namespace PlugThruDevice
