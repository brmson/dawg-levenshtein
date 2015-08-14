__author__ = 'vesely'

import pydawg

words = [u'foo', u'bar', u'baz', u'qux', u'quux', u'corge', u'grault', u'garply', u'waldo', u'fred', u'plugh', u'xyzzy',
         u'thud']
words.sort()

#build directed acyclic word graph
d = pydawg.PyDawg()
for word in words:
    d.insert(word)
d.finish()

#query for words within edit distance 2 of the word boz
print d.fuzzy_search(u"boz", 2)
