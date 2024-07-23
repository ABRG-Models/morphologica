#!/bin/bash

# Run the doc webpage locally

echo "Assuming you already installed a ruby environment, with a command like:"
echo "  sudo apt install ruby ruby-dev"
echo "AND assuming you already installed jekyll, etc with:"
echo "  gem install --install-dir ~/gems bundler jekyll jekyll-default-layout just-the-docs github-pages"
bundle exec jekyll serve --livereload &
