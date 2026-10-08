import random
import sys
random.seed(42)
count=int(sys.argv[1]) if len(sys.argv)>1 else 1_000_000
words=['ABCD','ABDULLAH','KABC','ABCAB','TABLE','BONJOUR','RESEAU','ELECTRIQUE','ALGORITHME','SYSTEME']
with open('texte.txt','w',encoding='utf-8') as f:
    for i in range(count):
        f.write(random.choice(words))
        f.write('\n' if i%20==19 else ' ')
print(f'{count} mots generes dans texte.txt')
