#include "navigator.hpp"

int main( int argc, char **argv )
{
	CS488Window::launch( argc, argv, new Navigator(), 1024, 768, "GPU Procedural Terrain 488" );
	return 0;
}
