NSIS 7z Extract Plugin.
(c) brainsucker (Nik Medved), 2009.
Uses 7-Zip source code, (C) 1999-2009 Igor Pavlov.

Abstract.
---------

Nowadays NSIS has very good compression algorithms (including superb 
lzma-solid), but neverless, dedicated archivers provide slightly better
ratios. Moreover, if you have "pack of files" which is never changed itself, 
but used in several different or frequent releases, it will be handy to pack
it once with 7z, and then use like "merge-module", to save several bytes 
and setup compilation time together (the last is most important, we get up to
10x compilation time reduction for some of our setups using 60mb 7z-packed (240
original) merge modules).

History.
--------

Actually we are using this plugin as binary for around 4 years (it was first 
compiled with something like 7z 4.05), but around 7z releases 4.30 it lost 
compatibility with the newer archives (so we kept in secret place special 
"old" 7z version to pack our merge-modules ;). And strangely, in several 
weeks after initial compilation I've lost my hard-drive, so we got only
this single "nsis7z.dll" file kept in our CVS for years. But due to 7z license
that wasn't completely legal, and also some other people were asking me for 
license details (to use this plugin in their products setups).

Yesterday I've got some spare time, and decided to recreate original plugin,
saving compatibility with old one, but adding some minor features, and 
trying to preserve source-code compatibility with future 7z-releases (as much
as I can).

Usage.
------

Simple. Look at example1.nsi.
Details? Unpack you merge module as 7z archive to somewere on users harddrive,
set target dir for archive contents with SetOutPath, and call plugin. Example:

	File "ArchiveName.7z"
	Nsis7z::Extract "ArchiveName.7z"
	Delete "$OUTDIR/ArchiveName.7z"

Usually I'm showing something like "Installing package..." with DetailsPrint 
before this, and plugin will move progress bar during unpack operation, to keep
user notified on setup progress.

Plugin now has two additional commands:

	Nsis7z::ExtractWithDetails "Test.7z" "Installing package %s..."

Unpacks archive in details mode, with promt generated from second param, use
 %s to insert unpack details like "10% (5 / 10 MB)"

  	GetFunctionAddress $R9 CallbackTest
  	Nsis7z::ExtractWithCallback "Test.7z" $R9

Unpacks archive in callback mode - plugin will animate progress bar, you can do
anything (like setting user prompt) in callback function.


License.
--------

I know very little about open source world and associated laws, but AFAIK ;) I
can compile, distribute and use this plugin under LGPL license (7z uses one) 
as soon as source code for it is readily available. No problems here, all 
regards and fame goes to Igor Pavlov, 7-zip author.

You can download source code for this plugin at http://nsis.sf.net or 
http://brains.by or ask me at any time to send it to you, if you got 
distribution package without sources (brainsucker.na@gmail.com).

Compiling.
----------

Sorry, but I have no time to recreate make files by hand, so only VS2008
solution/project are included. To build plugin you need to unpack 7z464.tar.bz2 
(or later, hopefully; you can retreive one from www.7-zip.org) somewhere, and 
then unpack plugin source distribution to the same folder. Solution for plugin
can be found at CPP\7zip\Bundles\Nsis7z subfolder (Nsis7z.sln). You should 
compile it as ANSI version (NOT unicode, Release or Debug).

I've tried to strip plugin as much as possible (still very big, 170 kb too much
comparing to 30 kb ansi-C 7z unpacker provided by Igor Pavlov), so it should 
support only 7Z unpacking, with LZMA, Deflate, BCJ2, etc. methods. Also it will
only work at NT systems (95/98/ME - bad luck, sorry ;), according to source code
and statements at comments.