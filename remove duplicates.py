# write a program to remove duplicate lines from a txt file using the open() function

file = open('songs/Playlist.csv', 'r')
lines = file.readlines()
file.close()

storage = []

for line in lines:
    test = line.split(',')
    test.pop(0)

    line = ','.join(test)

    storage.append(line)


removed = list(set(storage))

for line in storage:
    if line not in removed:
        print(line)

lines = [ str(i) + ',' + line for i, line in enumerate(removed) ]

file = open('duplicates removed.csv', 'w')
file.writelines(lines)
file.close()