Name:           foobillardplus
Version:        3.41beta
Release:        1.0.0

Group:          Amusements/Games/3D/Other
Summary:        A free OpenGL 3D billiard game

Vendor:         Holger Schaekel (foobillardplus@go4more.de)
Packager:       Holger Schaekel (foobillardplus@go4more.de)
URL:            http://foobillardplus.sourceforge.net
License:        GPL 2
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-root
#Requires:       libSDL, SDL_net, freetype2
#BuildRequires:  libSDL-devel, libSDL_net-devel, freetype2-devel

Prefix: /opt

%description
FooBillard++ is an attempt to create a free OpenGL-billard for Linux. Why foo?
Well, Florian Berger had this logo (F.B.-Florian Berger) and then foo sounds a
bit like pool (Somehow he wasn't quite attracted by the name FoolBillard)
If you are a billard-pro and you're missing some physics,
please tell us. Please contact us over sourceforge.net

Started at 12/2010 an advanced version is in progress with foobillard++.
It's not really a new game, but an advanced with a lot of fixes, new
options, graphics and features until the last version of foobillard (2007).
At this point it's started with a special version for the Touch-PC WeTab.

%prep
#Unpack package
%setup

%install
make install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%dir /opt/%{name}
%dir /opt/%{name}/bin
%dir /opt/%{name}/data
/opt/%{name}/data/*
/opt/%{name}/bin/*
/opt/%{name}/AUTHORS
/opt/%{name}/COPYING
/opt/%{name}/ChangeLog
/opt/%{name}/INSTALL
/opt/%{name}/README
/opt/%{name}/TODO
/opt/%{name}/foobillardplus.desktop
/opt/%{name}/foobillardplus.png
/opt/%{name}/foobillardplus.xbm

%build
./configure --prefix=$RPM_BUILD_ROOT/opt --enable-special
make 

%changelog
* Wed Nov 01 2011 Holger Schaekel
- Bugfixes with language handling
- res files for windows

* Sun Sep 26 2011 Robert Brandl, Holger Schaekel
- Port for MS-Windows
- Make 3.4 Source compatible for both worlds (Win/Linux)

* Wed Sep 14 2011 Holger Schaekel
- textures for furniture
- textures for fireplace
- new routines for blender exported meshes
- routines for display (OpenGL 1.4 - for WeTab compatibility)
- bugfixing for antialiasing for some Linux-Versions
- example foobillardplus.desktop file

* Fri Aug 26 2011 Holger Schaekel
- textures for room and walls
- routines for displaying walls
- more optimizations in table.c 

* Thu Jul 26 2011 Holger Schaekel
- Tron like game playfield (a little bit like the movie ;-)
- Glass balls (not really perfect, but a beginning
- new network module IPv4
- IPv6 only in the start mode shown, but only IPv4 works now (SDL_net problem)
- all game types can be played in the network gaming mode
- in birds eye view it's now possible to play a game with cue-moving etc.
- the main-menu now shows some more help on every option.

* Fri May 20 2011 Holger Schaekel
 - a little bit textures for the roomfloor
 - new handling of changing to the next shot (more human feeling)
 - start to write a whole manual
 - start of basic utf8 handling for strings and fonts
 - bugfixing
 - much code rewriting
 - most mouse-commands also with keyboard possible
 - some problems with the .rc file solved
 - new improved controls direct on the screen (visible with icons)
 - some antialiasing filtering functionality
 - some anisotropic filtering funcionality
 - lost balls from table are detected correctly in jump shots
 - switching the jump shots on as default
 - more graphics
 - more sound
 - more options adjustable inside the game
 - new options
 - carambol play has now a (adjustable) defined ending with winning option
 - carambol play has a functional tournament mode
 - bug fixing
 - SDL is now a must have. Glut isn't supported anymore
 - freetype2 is now a must have. Without it, the program won't work
 - Snooker shows now the correct balls to play
 - new statusline for more info in gameplay

* Mon Jan 24 2011 Holger Schaekel
 - Start of Localisation
 - better orientation for the Fonts
 - changed fonts for blueboldttf,iomanoid,youregon because they are not GPL
   (thanks to the Meego Port Team)
 - Special Options for WeTab Port
 - Corrected Game Start
 - Corrected Tournament
 - Corrected functions in billard3d.c
 - Corrected Menu handling
 - Improved and corrected moving around in all states
 - program not published

* Thu Jan 18 2011 Holger Schaekel
- start of the first playable version foobillard++
- remove manpage, build html manual
- program not published

* Sun Jan 18 2003 Florian Berger <harpin_floh@yahoo.de>
- added manpage

* Thu May 16 2002 Michal Ambroz (O_O) <rebus@seznam.cz>
- spring cleanup 

* Fri Mar 08 2002 Michal Ambroz (O_O) <rebus@seznam.cz>
- initial foobillard specfile
