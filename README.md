darwin
======

track changes as a genome evolves over time (hence the name)

##### intent:
* merge automatic annotation and version control technologies
	* this was benchling's pitch but they're not open source
* if putting in some interim layer above git turns out to work really well, this could be extensible to essentially any data, not just DNA

# MVP
1. (user) easy, standardized, useful collaborative tracking of changes
2. (user) entire history and changelog easily transferable from place to place
3. (vendor) diffs compressed immensely compared to naive methods
4. (vendor) security increased through hash-checking and decentralized control
    * decentralized: if a single server with data is compromised, other mirrors can convene and determine which is truly correct; like bitcoin's model.
* it's not like git, because it's specially made for genomic data (git is not granular enough)
* it's not like existing tools, because they don't have the kind of itemized change tracking that git does (existing tools are too granular) (or just nonfree)


## licensing (THIS IS REALLY IMPORTANT IF WE WANT THIS TO EVER GET OFF THE GROUND)
* [reference](http://www.gnu.org/licenses/gpl.html)
* git is licensed under the GPL v2 (and other compatible licenses but mostly that)
* however, if this software does not modify the git code, but merely creates some layer to call git on (unmodified, without dynamically sharing data), then we're in the clear
* consider merits of gpl, though:
	* so if some corporation wishes to use this software commercially, they're fine, as long as they don't modify it on their own system and offer that version to consumers
	* they'll just need to add modifications back into the original codebase, which is actually totally great
	* the automatic annotation engine might also be licensed

## current next steps
1. need automatic annotation engine (currently focusing just on prokaryotes, or if required even just E.coli/plasmids. we need to get this out /fast/)
    * [reference](http://en.wikipedia.org/wiki/Genome_project#Genome_annotation)
    * more:
        * http://www.hsls.pitt.edu/obrc/index.php?page=genome_browsers_annotation_analysis (like a million items)
        * ensembl is only for eukaryotic cells and is therefore useless
    * ideas for integrating annotation:
        * delimit with newlines all recognized codon sequences
        * have unrecognized or junk contiguous sequences on a newline
        * need method of merging single-character (or multiple-character!) mutations without freaking out
    * 'BLAST' is method of determining DNA sequence similarity to existing sequences, used in most auto-annotation programs
    * [NCBI Prokaryotic Genome Annotation Pipeline](http://www.ncbi.nlm.nih.gov/genome/annotation_prok/)
        * not usable for our purposes (not exportable), but documentation as to how it's done is available, and may be useful
2. need to develop method of integration with git
    1. can add a hook onto the diff method, and all other appropriate methods, which converts file into delimited format according to annotation
        * would need to ensure that files are kept the same on disk so that ApE/whatever can access them easily and biologists aren't confused
            * this would probably require being able to convert to/from the annotated form upon git diffing
        * especially given the above there'll be major performance issues depending upon the automatic annotation library used
        * we CANNOT use something that goes out to the web to check up on data; if necessary we'll keep all the annotation data locally and update it when the application is updated
            * well....................maybe we can, if we cache the auto-annotation procedure so that it runs a very small number of times, and only lengthy when the file is first added
    2. should keep in mind the ability to work with git gui, too; no need to remake yet another wheel and biologists don't like command lines
    3. each individual line is gonna be insanely long (gfp is 238 amino acids, for example, in a single ORF) compared to standard diff, even with annotation and split by ORF
		* potential methods then include marking each individual character change
		* this could potentially get very slow very quickly when attempting to apply diffs

## thought process:
1. stick this onto git somehow. we're not going to reinvent an incredibly well-crafted wheel.
2. git is great at line-based differentation. git is not great at dealing with a single very long string i.e. DNA data.
3. we need some way to differentiate between different logical parts of DNA so we can diff at the finest possible granularity.
4. wait...................AUTOMATIC ANNOTATION!!! extend this idea to split DNA semantically as best as possible

## questions/notes:
* do we need to fundamentally modify git itself, or can we just wrap git with a few scripts and therefore just use the copy of git already installed?
    * this would mean we wouldn't need to release under the GPL; we'll have to see where that goes
* do we assume the user will track only a single file at a time? because plasmid files are typically monolithic and don't rely upon one another? does that matter?
* do we attempt to diff by character as well, if a user wishes to make a line-item mutation?
* which filetypes do we support?
* will automatic annotation become incredibly cumbersome to use?
* we need to allow switching out annotation software at will, in a modular fashion
    * let's only support a single one right now, but let's write the code so that it doesn't rely on the characteristics of that single program

