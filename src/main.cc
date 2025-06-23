#include "vchambers.hh"

int main() {
	vc::VChambers vchambers;

	vchambers.initalize();
	while (true) {
		vchambers.render();
	}
	vchambers.end();
}
