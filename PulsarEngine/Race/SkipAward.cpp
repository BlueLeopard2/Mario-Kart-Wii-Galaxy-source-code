#include <PulsarSystem.hpp>

namespace Pulsar {
namespace IKW {

//Load Credits [CLF78]
kmCallDefAsm(0x8085AFEC) {
loc_0x0:
		mr        r4, r31
		subi      r5, r31, 0x35
		cmplwi    r5, 0x3
		bgt - loc_0x14
		li        r4, 0x41

		loc_0x14:

}
}//namespace IKW
}//namespace Pulsar
