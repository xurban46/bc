# Colearning in Coevolutionary Algorithms
# Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>
#
# Master Thesis
# 2014/2015
#
# Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>
#
# Faculty of Information Technologies
# Brno University of Technology
# http://www.fit.vutbr.cz/
#
# Started on 28/07/2014.
#      _       _
#   __(.)=   =(.)__
#   \___)     (___/


# _XOPEN_SOURCE=700 required by scandir and alphasort from vault.c

CC=gcc
CFLAGS=-g -Wall -std=c11 -fopenmp -O0 -D_XOPEN_SOURCE=700 \
	-DSSE2 -DxAVX2 -DDEBUG -DxVERBOSE -DxCGP_LIMIT_FUNCS
LIBS=-lm -lc

SOURCES=main.c cpu.c ga.c cgp/cgp_core.c cgp/cgp_dump.c cgp/cgp_load.c cgp/cgp_avx.c cgp/cgp_sse.c \
	predictors.c image.c fitness.c fitness_avx.c fitness_sse.c \
	archive.c config.c algo.c baldwin.c utils.c \
	logging/history.c logging/base.c logging/text.c logging/csv.c logging/summary.c

EXECUTABLE=coco
OFILES= main.o cpu.o ga.o cgp/cgp_core.o cgp/cgp_dump.o cgp/cgp_load.o cgp/cgp_avx.o cgp/cgp_sse.o \
	predictors.o image.o fitness.o fitness_avx.o fitness_sse.o \
	archive.o config.o algo.o baldwin.o utils.o \
	logging/history.o logging/base.o logging/text.o logging/csv.o logging/summary.o

EXECUTABLE_APPLY=coco_apply
OFILES_APPLY= image.o ga.o cgp/cgp_core.o cgp/cgp_load.o main_apply.o

CMDLINE=-i ../images/lena_gray_256.png -n ../images/lena_gray_256_saltpepper_15.png -g 10000 -a cgp -S 100 -I 25 -k 10000
ANSELM_HOST=anselm
ANSELM_PATH=~/xwigla00
MERLIN_HOST=merlin
MERLIN_PATH=~/coco

.PHONY: clean run minirun zip tar upload start depend anselmup anselmdown merlinup rebuild callgraph

all: $(EXECUTABLE) $(EXECUTABLE_APPLY)

clean:
	rm -f *.o cgp/*.o logging/*.o
	rm -rf cocolog/*
	rm -f $(EXECUTABLE) $(EXECUTABLE).exe $(EXECUTABLE).exe.stackdump
	rm -f $(EXECUTABLE_APPLY) $(EXECUTABLE_APPLY).exe $(EXECUTABLE_APPLY).exe.stackdump
	rm -f xwigla00.zip xwigla00.tar.gz
	rm -f *.expand cgp/*.expand logging/*.expand

rebuild: clean all

$(EXECUTABLE): $(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES) $(LIBS)

$(EXECUTABLE_APPLY): $(OFILES_APPLY)
	$(CC) $(CFLAGS) -o $(EXECUTABLE_APPLY) $(OFILES_APPLY) $(LIBS)

run: $(EXECUTABLE)
	rm -rf cocolog/*
	./$(EXECUTABLE) $(CMDLINE)

minirun: $(EXECUTABLE)
	rm -rf cocolog/*
	./$(EXECUTABLE) -i ../images/10x10.png -n ../images/10x10_sp25.png -a baldwin -g 1000 -S 75 -I 50 -N 50

tar:
	cd .. && tar czf xwigla00.tar.gz src/*.c src/*.h src/Makefile src/tests/* src/stb/* # images/* #-C doc/ doc.pdf

anselmup: tar
	find -type f | xargs chmod ug+rw
	find -type d | xargs chmod ug+rwx
	scp ../xwigla00.tar.gz $(ANSELM_HOST):$(ANSELM_PATH)
	ssh $(ANSELM_HOST) "cd $(ANSELM_PATH) && tar -xzvf xwigla00.tar.gz && cd src && module load gcc/4.9.0 && make clean all"

anselmdown:
	ssh $(ANSELM_HOST) "cd $(ANSELM_PATH)/results && tar -czvf ../results.tar.gz --exclude='img_filtered_*.png' *"
	scp $(ANSELM_HOST):$(ANSELM_PATH)/results.tar.gz ../anselm/
	cd ../anselm/ && tar -xzvf results.tar.gz

merlinup: tar
	find -type f | xargs chmod ug+rw
	find -type d | xargs chmod ug+rwx
	scp ../xwigla00.tar.gz $(MERLIN_HOST):$(MERLIN_PATH)
	ssh $(MERLIN_HOST) "cd $(MERLIN_PATH) && tar -xzvf xwigla00.tar.gz && cd src && make clean all"

# AVX2 support

cgp/cgp_avx.o: cgp/cgp_avx.c
	$(CC) $(CFLAGS) -mavx2 -c $< -o $@

fitness_avx.o: fitness_avx.c
	$(CC) $(CFLAGS) -mavx2 -c $< -o $@

# SSE2 support

cgp/cgp_sse.o: cgp/cgp_sse.c
	$(CC) $(CFLAGS) -msse2 -c $< -o $@

fitness_sse.o: fitness_sse.c
	$(CC) $(CFLAGS) -msse2 -c $< -o $@

# some stuff to increase average developer happiness

plot:
	gnuplot cocolog.gp

valgrind: $(EXECUTABLE)
	rm -rf cocolog/*
	valgrind --track-origins=yes --leak-check=full ./$(EXECUTABLE) $(CMDLINE)

leaks: $(EXECUTABLE)
	rm -rf cocolog/*
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./$(EXECUTABLE) $(CMDLINE)

callgraph:
	$(MAKE) CFLAGS='$(CFLAGS) -fdump-rtl-expand' clean all
	find -name '*.expand' | grep -v 'main_apply' | xargs egypt --omit stbi_load,stbi_write_png,log_entry_prolog,can_use_sse2 | dot -Grankdir=LR  -Tpng -o callgraph.png
	rm *.expand cgp/*.expand logging/*.expand

# general rules

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# dependencies

depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $(SOURCES) >>./.depend;

include .depend

