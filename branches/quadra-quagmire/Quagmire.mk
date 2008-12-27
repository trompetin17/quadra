# -*- makefile-gmake -*-

bin_PROGRAMS = source/quadra
noinst_PROGRAMS += skelton/tools/wadder/wadder
noinst_PROGRAMS += skelton/tools/dumpwad
dataroot_DATA = quadra.res

CPPFLAGS+=-Iinclude -I$(srcdir)/include -I$(srcdir)/skelton/include

quadra.res: $(srcdir)/resources.txt skelton/tools/wadder/wadder $(shell cat $(srcdir)/resources.txt)
	skelton/tools/wadder/wadder $(srcdir) $@ $<

source/quadra: CPPFLAGS+=-DDATAGAMESDIR=\"$(datadir)/games\"
source/quadra: LDFLAGS+=$(SDL_LIBS) -lz -lpng

source/quadra_SOURCES += $(SDL_MAIN_SRCS)

source/quadra_SOURCES += skelton/common/bitmap.cpp
source/quadra_SOURCES += skelton/common/buf.cpp
source/quadra_SOURCES += skelton/common/clipable.cpp
source/quadra_SOURCES += skelton/common/clock.cpp
source/quadra_SOURCES += skelton/common/command.cpp
source/quadra_SOURCES += skelton/common/crypt.cpp
source/quadra_SOURCES += skelton/common/cursor.cpp
source/quadra_SOURCES += skelton/common/dict.cpp
source/quadra_SOURCES += skelton/common/http_post.cpp
source/quadra_SOURCES += skelton/common/http_request.cpp
source/quadra_SOURCES += skelton/common/id.cpp
source/quadra_SOURCES += skelton/common/image_png.cpp
source/quadra_SOURCES += skelton/common/input_dumb.cpp
source/quadra_SOURCES += skelton/common/inter.cpp
source/quadra_SOURCES += skelton/common/listbox.cpp
source/quadra_SOURCES += skelton/common/net.cpp
source/quadra_SOURCES += skelton/common/notify.cpp
source/quadra_SOURCES += skelton/common/overmind.cpp
source/quadra_SOURCES += skelton/common/packet.cpp
source/quadra_SOURCES += skelton/common/random.cpp
source/quadra_SOURCES += skelton/common/res_compress.cpp
source/quadra_SOURCES += skelton/common/resfile.cpp
source/quadra_SOURCES += skelton/common/resmanager.cpp
source/quadra_SOURCES += skelton/common/reswriter.cpp
source/quadra_SOURCES += skelton/common/sprite.cpp
source/quadra_SOURCES += skelton/common/stringtable.cpp
source/quadra_SOURCES += skelton/common/unicode.cpp
source/quadra_SOURCES += skelton/common/url.cpp
source/quadra_SOURCES += skelton/common/zone_text_clock.cpp
source/quadra_SOURCES += skelton/svgalib/error.cpp
source/quadra_SOURCES += skelton/svgalib/find_file.cpp
source/quadra_SOURCES += skelton/svgalib/input.cpp
source/quadra_SOURCES += skelton/svgalib/main.cpp
source/quadra_SOURCES += skelton/svgalib/palette.cpp
source/quadra_SOURCES += skelton/svgalib/res.cpp
source/quadra_SOURCES += skelton/svgalib/sound.cpp
source/quadra_SOURCES += skelton/svgalib/video.cpp
source/quadra_SOURCES += source/bloc.cpp
source/quadra_SOURCES += source/canvas.cpp
source/quadra_SOURCES += source/cfgfile.cpp
source/quadra_SOURCES += source/chat_text.cpp
source/quadra_SOURCES += source/color.cpp
source/quadra_SOURCES += source/fonts.cpp
source/quadra_SOURCES += source/game.cpp
source/quadra_SOURCES += source/game_menu.cpp
source/quadra_SOURCES += source/global.cpp
source/quadra_SOURCES += source/highscores.cpp
source/quadra_SOURCES += source/menu.cpp
source/quadra_SOURCES += source/menu_base.cpp
source/quadra_SOURCES += source/menu_demo_central.cpp
source/quadra_SOURCES += source/misc.cpp
source/quadra_SOURCES += source/multi_player.cpp
source/quadra_SOURCES += source/net_list.cpp
source/quadra_SOURCES += source/net_server.cpp
source/quadra_SOURCES += source/net_stuff.cpp
source/quadra_SOURCES += source/nglog.cpp
source/quadra_SOURCES += source/packets.cpp
source/quadra_SOURCES += source/pane.cpp
source/quadra_SOURCES += source/qserv.cpp
source/quadra_SOURCES += source/quadra.cpp
source/quadra_SOURCES += source/recording.cpp
source/quadra_SOURCES += source/score.cpp
source/quadra_SOURCES += source/sons.cpp
source/quadra_SOURCES += source/stats.cpp
source/quadra_SOURCES += source/update.cpp
source/quadra_SOURCES += source/zone.cpp

skelton/tools/wadder/wadder_SOURCES += skelton/common/resfile.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/common/resmanager.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/common/reswriter.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/common/stringtable.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/svgalib/error.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/svgalib/res.cpp
skelton/tools/wadder/wadder_SOURCES += skelton/tools/wadder/wadder.cpp

skelton/tools/dumpwad_SOURCES += skelton/common/resfile.cpp
skelton/tools/dumpwad_SOURCES += skelton/common/reswriter.cpp
skelton/tools/dumpwad_SOURCES += skelton/svgalib/error.cpp
skelton/tools/dumpwad_SOURCES += skelton/svgalib/res.cpp
skelton/tools/dumpwad_SOURCES += skelton/tools/dumpwad.cpp

