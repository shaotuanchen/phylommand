Here are some examples on how phylommand can be used. The examples assumes a
basic knowledge in how to work on the command line, for example that > will pipe
the output from stdout (the screen) to a file which is the standard way of
getting the output to a file in phylommand, e.g.:

    treebender --output svg tree_file.tree > tree_file.svg
 
Many cases also assume that you are working in a bash shell or similar.

To make and view a neighbour joining tree based on a mafft alignment:

    mafft alignment_file.fst | pairalign -A -j -n -m | treeator -n | treebender \
    --output svg | rudisvg

To create monophyletic OTUs based on the branch lengths in a tree (c.f. virtual
taxa sensu �pick et al 2009):

    treebender --cluster branch_length:0.03 tree_file.tree 

Get the parsimony score of all nearest neighbour interchange trees from:

    treebender --nni all tree_file.tree | treeator -f alignment_file.fst -p

Get alignable groups based on MAD score (Smith et al. 2009, BMC Evol. Biol.
9:37):

    pairalign --group alignment_groups alignment_file_with_taxon_string.fst

The MAD score does not care about gaps, this means that if the pairwise
alignments are over aligned (inserting more gaps than there should be to match
the same type of bases to each other), alignment_groups may over estimate what
can be aligned. Sometimes a multiple sequence alignment may be better in this
respect, and programs like, for example, muscle and mafft may even be faster
than pairalign in aligning. So it is possible to do something like:

    mafft alignment_file_with_taxon_string.fst | pairalign --group \
    alignment_groups -A

Delete old support values, and draw new one on a topology:

    treebender --clear_internal_node_labels tree_file.tree | contree -d \
    tree_file.trees.nex -s

Get conflict with supported above 70 based on an interval of 100 trees, compared
the support in a given tree, output in HTML format:

    treebender --interval 533-632 tree_file.trees.nex | contree -d - -a \
    tree_file.nex | contree -c 70 -d tree_file.tree --html

Get the maximum clade credibility tree in a set of trees, removing burn-in (5000
trees), and with help of awk:

    treebender --interval 5001 tree_file.trees | contree -a | treebender \
    --clade_credibility | awk -v max=0 '{if($1>max){print NR " " $1; max=$1}}'

The first value is which tree after burn-in, and the second the log sum of
support values. The last printed values should be for the maximum clade
credibility (MCC) tree. The second value is not the clade credibility since we
do not divide the support by the number of trees.

If we want to change the names of many or all tips in a tree it may be convenient 
to use a file with the name changes. We can get a nice start for such a file
getting the tip names from treebender:

    treebender -t '|,\n' tree_file.tree > name_changes.txt

We can then then edit name_changes.txt, putting our new names between the '|' and
',' (you need to add | for the last name though; depending on your system and
text editor you may want to use other line break than \n), and then run:

    treebender -c file:name_changes.txt tree_file.tree

Do not forget to add new name or delete the row if you do not want to change that
name, otherwise these tip names will be deleted. If you like the output in svg
format just add --output svg. Start files for the other arguments that take
taxa as input can be gotten similarly:

    treebender -t ,\\n tree_file.tree > taxa.txt

To be able to pipe all information for the --group analyses it is possible to
give the taxonomy in the sequence names. However, pairalign does not output the
taxonomy as part of the sequence name. This may be irritating sometimes when you
make a pairwise alignment to use for a later --group analysis. To get the
taxonomy from the first fasta file into a separate file that can be used for the
analysis you can do:

    grep '>' alignment_file_with_taxon_string.fst | sed 's/^>//' | awk \
    'BEGIN { FS="|" } {print $2 "|" $1}' > taxonomy_file.txt

If you want to summarize the rates in a MCC tree generated in with the BEAST
package you can do:

    treebender --internal_node_stats 1:rate_median tree_file.nex

It is also possible to do:

    treebender --internal_node_stats 1:rate tree_file.trees.nex

if you want to look at the rates over all trees in the tree distribution
produced by BEAST. This does however quickly become incomprehensible, so you
will probably need a script to do something meaningful with the output. Tracer
is probably also an better alternative, in most cases.

To reconstruct the ancestral state of the first trait in a DNA data matrix you
can do:

    treeator -a dna -t tree_file.tree --get_state_at_nodes \
    -l alignment_file.fst

If you want to make it a one rate model you can do:

    treeator -a dna -t tree_file.tree --get_state_at_nodes -l \
    -m 0,0,0,0,0,0,0,0,0,0,0,0 alignment_file.fst

or as an symmetric model:

    treeator -a dna -t tree_file.tree --get_state_at_nodes -l \
    -m 0,1,2,0,3,4,1,3,5,2,4,5 alignment_file.fst

Notice that these are not time reversible models. You may change the alphabet
given to -a if you are working with other type of characters.

