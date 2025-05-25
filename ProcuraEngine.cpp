// ProcuraEngine.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <locale>

#include "Nex.h"

int main(int argc, char* argv[])
{
	CNex nex;

	std::locale::global(std::locale(""));

	nex.TesteManual("Jogo Nex");
}
