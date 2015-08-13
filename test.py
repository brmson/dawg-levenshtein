__author__ = 'vesely'
import pyximport
pyximport.install()

from fuzzy import Dawg

words = [u'foo', u'bar', u'baz', u'qux', u'quux', u'corge', u'grault', u'garply', u'waldo', u'fred', u'plugh', u'xyzzy',
         u'thud']
words.sort()

#build directed acyclic word graph
dawg = Dawg()
for word in words:
    dawg.insert(word)
dawg.finish()

#query for words within edit distance 2 of the word boz
print dawg.fuzzy_search(u"boz", 2)
