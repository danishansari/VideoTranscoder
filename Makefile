CXX = -g -Wall
SRCDIR = src
OBJDIR = obj
BINDIR = bin

FFMPEG_2_7_6_SUPPORT = yes 

SRCS = VideoDecoder.cpp VideoEncoder.cpp
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

LIBS = avcodec avformat avutil swscale
LIBDIRS = /opt/ffmpeg-2.7.6/lib/
LIBFLAGS = $(LIBS:%=-l%)
LIBDIRFLAGS = $(LIBDIRS:%=-L%)

INCFLAGS = -Iinclude -I/opt/ffmpeg-2.7.6/include -DFFMPEG_2_7_6 


BIN_TRGTS = $(BINDIR)/testTranscode

LDFLAGS = $(LIBFLAGS) $(LIBDIRFLAGS)
CXXFLAGS = $(INCFLAGS)

all: $(BIN_TRGTS)

$(BIN_TRGTS): $(OBJS)
	@mkdir -p $(@D)
	g++ $(CXX) test/testTranscoding.cpp $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	g++ $(CXX) -c -Iinclude $< -o $@ $(CXXFLAGS)

clean:
	rm -f $(OBJDIR)/*.o $(BIN_TRGTS)
