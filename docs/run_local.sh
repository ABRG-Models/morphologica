#!/bin/bash

# Run the doc webpage locally

echo "Assuming you already installed with:"
echo "  sudo gem install bundler jekyll jekyll-default-layout just-the-docs github-pages"
bundle exec jekyll serve --livereload &
