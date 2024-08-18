# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'libfyaml'
copyright = '2019, Pantelis Antoniou'
author = 'Pantelis Antoniou'

# The full version, including alpha/beta/rc tags
release = '0.1'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'linuxdoc.rstFlatTable'    # Implementation of the 'flat-table' reST-directive.
  , 'linuxdoc.rstKernelDoc'    # Implementation of the 'kernel-doc' reST-directive.
  , 'linuxdoc.kernel_include'  # Implementation of the 'kernel-include' reST-directive.
  , 'linuxdoc.manKernelDoc'    # Implementation of the 'kernel-doc-man' builder
  , 'linuxdoc.cdomain'         # Replacement for the sphinx c-domain.
  , 'linuxdoc.kfigure'         # Sphinx extension which implements scalable image handling.
  , 'sphinx_markdown_builder'  # Sphinx markdown builder
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
# html_theme = 'alabaster'
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

master_doc = 'index'

man_pages = [
    ('man/fy-tool', 'fy-tool', 'fy-tool documentation ', '', 1),
]
