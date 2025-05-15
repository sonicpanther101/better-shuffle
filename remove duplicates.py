# write a program to remove duplicate lines from a txt file using the open() function

file = open('songs/Adams Playlist #5326.csv', 'r')
lines = file.readlines()
lines = list(set(lines))
file.close()

file = open('duplicates.csv', 'w')
file.writelines(lines)
file.close()