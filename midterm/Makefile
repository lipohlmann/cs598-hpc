include ../common.mk

# ---- Assignment-specific settings ----
TARGET  = midterm
SRCS    = $(wildcard src/*.c)
OBJS    = $(SRCS:.c=.o)

# ---- gslib (git submodule) ----
GSLIB_DIR = external/gslib
GSLIB_LIB = $(GSLIB_DIR)/build/lib/libgs.a
# -isystem silences -Wall/-Wextra noise from gslib's own headers
CFLAGS   += -isystem $(GSLIB_DIR)/build/include

PRESENTATION  = presentation/presentation.tex

.PHONY: all code pdf clean distclean run gslib

all: code pdf

code: $(TARGET)

gslib: $(GSLIB_LIB)

$(GSLIB_LIB):
	$(MAKE) -C $(GSLIB_DIR) CC=$(CC) CFLAGS=-O2 MPI=1

# gslib headers must be installed before any object is compiled
$(OBJS): | $(GSLIB_LIB)

$(TARGET): $(OBJS) $(GSLIB_LIB)
	$(CC) $(OBJS) $(GSLIB_LIB) -o $@ $(LDFLAGS)

pdf:
	$(LATEXMK) $(LATEXOPTS) -cd $(PRESENTATION)

run: code
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
	$(LATEXMK) -C -cd $(PRESENTATION)

distclean: clean
	$(MAKE) -C $(GSLIB_DIR) clean
	rm -rf $(GSLIB_DIR)/build
