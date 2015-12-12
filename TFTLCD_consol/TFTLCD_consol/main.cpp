#include "tftlcd.h"
#include <conio.h>

int main(void) {
	PanelImage pi;
	pi.openImage(String("000012.bmp"));
	pi.autoDefectSearch();

	return 0;
}