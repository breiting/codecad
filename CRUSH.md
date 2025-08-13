# CRUSH Guide for This Codebase

## Commands

- **Build:**
  ```bash
  mkdir -p build && cd build && cmake .. && make
  ```
- **Lint:**
  ```bash
  clang-format -i src/**/*.cpp
  ```
- **Run Tests:**
  ```bash
  ctest
  ```
- **Run a Single Test:**
  ```bash
  ctest -R test_name
  ```

## Code Style Guidelines

- **Imports:**
  - Group imports logically: standard libraries first, then third-party, then local modules.
- **Formatting:**
  - Use 4 spaces for indentation.
  - Lines should not exceed 80 characters.
- **Types:**
  - Prefer using `auto` where types are clear.
  - Use consistent naming for types (PascalCase for types).

- **Naming Conventions:**
  - Variables: `lowerCamelCase`
  - Public Functions/Methods: `PascalCase`
  - Private Functions/Methods: `lowerCamelCase`
  - Classes: `PascalCase`
- **Error Handling:**
  - Use exceptions for handling errors.
  - Ensure all exceptions are adequately caught and logged.

## Additional Guidelines

- Ensure all new code is tested and passes existing tests.
- Review and update documentation for any significant changes made in the code.

