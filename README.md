This repository contains two tools for split manufacturing, DEF\_RT and Splitter, in different branches: DEF\_RT is in "rt", Splitter in
"splitter" The master branch relates to "rt".

DEF\_RT: converting all the regular nets of a DEF file into the .rt format
DEF\_RT:  performs also splitting
-------------------------------------------------------------------------
IO> Usage: ./DEF-RT DEF\_file
IO>
IO> Mandatory parameter ``DEF\_file'': the DEF file to be converted
IO> Optional parameter ``nets\_file'': the file containing the nets to be considered; if none given, all nets are considered

DEF\_RT is a custom splitter and file converte for the routing attack (J. Magaña, D. Shi, and A. Davoodi. Are proximity attacks a threat to the
security of split manufacturing of integrated circuits? In International Conf.  on Computer-Aided Design, pages 1-7, 2016.) See
the folder attack/ for details on that.

Splitter: splitting a DEF file into FEOL/BEOL, beyond a given metal layer
-------------------------------------------------------------------------
IO> Usage: ./Splitter DEF\_file LEF\_file split\_layer
IO>
IO> Mandatory parameter ``DEF\_file'': the DEF file to be split
IO> Mandatory parameter ``LEF\_file'': the LEF file related to the DEF file
IO> Mandatory parameter ``split\_layer'': the layer after which to split the DEF file -- provide string, e.g., metal2

Splitter is a generic tool to split a DEF. The code as NOT done; as of now, only the DEF parsing is there, the splitting and writing back to DEF
is missing. See also TODOs in the code.
