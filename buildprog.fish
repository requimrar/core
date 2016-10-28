#!/usr/local/bin/fish



function callCompiler

	rm -r build/sysroot/usr/local/lib/flaxlibs/*
	cp -R libs/* build/sysroot/usr/local/lib/flaxlibs/

	set opt "-O3"
	set dump ""
	set compile "-jit"

	if contains "noopt" $argv
		set opt "-O0"
	end

	if contains "dump" $argv
		set dump "-print-lir"
		set compile "-jit"
	else if contains "dumpf" $argv
		set dump "-print-fir"
		set compile "-jit"
	else if contains "compile" $argv
		set dump ""
		set compile "-o build/test"
	else if contains "nolink" $argv
		set dump ""
		set compile "-c"
	end


	# copy the libraries
	eval mkdir -p "build/sysroot/usr/local/lib/flaxlibs/"
	eval cp "-R" "libs/*" "build/sysroot/usr/local/lib/flaxlibs/"


	eval time "build/sysroot/usr/local/bin/flaxc -Wno-unused-variable -sysroot build/sysroot" $opt $compile $dump "build/test.flx"
end


callCompiler $argv
