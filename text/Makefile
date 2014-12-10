# makefile pro preklad LaTeX verze Bc. prace
# (c) 2008 Michal Bidlo
# E-mail: bidlom AT fit vutbr cz
#===========================================
# asi budete chtit prejmenovat:
CO=projekt

.PHONY: clean pack

all: $(CO).pdf

pdf: $(CO).pdf

literatura-latin2.bib: literatura.bib
	iconv -f utf-8 -t latin2 literatura.bib > literatura-latin2.bib

$(CO).ps: $(CO).dvi literatura-latin2.bib
	dvips $(CO)

$(CO).pdf: clean literatura-latin2.bib
	pdflatex $(CO)
	bibtex $(CO)
	pdflatex $(CO)
	pdflatex $(CO)

$(CO).dvi: $(CO).tex literatura-latin2.bib
	latex $(CO)
	bibtex $(CO)
	latex $(CO)
	latex $(CO)

desky:
#	latex desky
#	dvips desky
#	dvipdf desky
	pdflatex desky

clean:
	rm -f *.dvi *.log $(CO).blg $(CO).bbl $(CO).toc *.aux $(CO).out $(CO).lof
	rm -f $(CO).pdf
	rm -f *~
	rm -f literatura-latin2.bib

pack:
	tar czvf dp-xwigla00.tar.gz *.tex *.bib *.bst ./fig/* ./cls/* Makefile Changelog
