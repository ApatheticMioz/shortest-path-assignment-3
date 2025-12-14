# Contributing to Shortest Path Algorithms Benchmark

Thank you for your interest in contributing to this project!

## How to Contribute

### Reporting Issues

- Use the GitHub issue tracker to report bugs
- Describe the issue in detail, including steps to reproduce
- Include your environment details (OS, compiler version)

### Submitting Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Ensure your code compiles without warnings:
   ```bash
   g++ -std=c++17 -Wall -Wextra -O2 shortest_path.cpp -o shortest_path
   ```
5. Test your changes
6. Commit with clear, descriptive messages
7. Push to your fork and submit a pull request

### Code Style

- Use 4-space indentation
- Follow existing code style and conventions
- Keep lines under 120 characters when practical
- Comment complex algorithms and non-obvious code

### Testing

- Test with various graph configurations (sparse, dense, mixed)
- Ensure no memory leaks or undefined behavior
- Verify output correctness against known test cases

## Code of Conduct

- Be respectful and constructive in discussions
- Focus on the code, not the person
- Help others learn and improve

## Questions?

Open an issue for any questions about contributing.
