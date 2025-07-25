# write a program to remove duplicate lines from a txt file using the open() function

file = open('songs/Playlist.csv', 'r')
lines = file.readlines()
file.close()

removed = []
storage = []

for line in lines:
    test = line.split(',')
    test.pop(0)

    line = ','.join(test)

    if line in storage:
        removed.append(line)
    else:
        storage.append(line)

# print difference between the two lists
print(len(removed))

lines = [ str(i) + ',' + line for i, line in enumerate(storage) ]
lines[0] = lines[0].replace("0","#")

file = open('duplicates removed.csv', 'w')
file.writelines(lines)
file.close()