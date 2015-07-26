This is a C++ ORM library that is written entirely in C++98 and doesn't require any external tool to translate model descriptions to C++ code. Models are described with C++ macros and later expanded to C++ classes. All calculations of required memory sizes are performed with template metaprogramming.

Example of a model definition:

```C++
BEGIN_MODEL(Author)
    FIELD(name, cot::StringValue<256>)
    FIELD(age, cot::IntValue)
    FIELD(bookCount, cot::IntValue)
END_MODEL
```

Examples of queries:

```C++
boost::scoped_ptr<Author> author(new Author());
author->name = "John Smith";
author->age = 19;
author->bookCount = 0;
author->save();

vector< boost::shared_ptr<Author> > authors = Select< Author, All >::with(0);
for (int i = 0; i < (int)authors.size(); ++i) {
    authors[i]->age += 1;
    authors[i]->save();
}

authors = Select<Author, Where<Or<Lt<Author::_age_>, Eq<Author::_name_> > > >::with(2, 41, "John Smith");

authors = Select< Author, Where< Eq<Author::_bookCount_> > >::with(1, 0);
for (int i = 0; i < (int)authors.size(); ++i) {
    authors[i]->remove();
}
```

The project is currently in clean-up mode.

Please, email d.a.stephantsov@gmail.com if you have any questions or suggestions.
