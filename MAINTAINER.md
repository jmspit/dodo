# Maintainer manual

## Coding style {#coding_style}

[TOC]

### Identifiers

#### Namespaces

Namespace identifiers are lower case.

```C
namespace dodo::network {
}
```

#### Type names

Type names start with a capital.

```C
class Person {
};
```

Each noun within the type name starts with a capital, composite nouns each start with a capital.

```C
class Logfile {
};

class LogfileAppender {
}
```

Acronyms and such preserve case

```C
class SMTPServer {
}
```

### Variables

Variables are lowercase.

```C
void add( int quantity ) {
}
```
Protected and private class attributes are suffixed with an underscore (`_`)

```C
class Person {
  protected:
    Date birthdate_;
};
```

#### Methods

```C
int getAttrribute() const {
}
```

### Indentation and curly brackets

No tabs. Indent is two spaces.

```C
void foo( const Collection &collection ) {
  if ( i.size() ) {
    for ( auto i : collection ) {
    }
  else {
    cout << "the collection is empty" << endl;
  }
}
```
