try:
    from .dependencies.AST.install import *
except ImportError:
    try:
        from dependencies.AST.install import *
    except ImportError:
        print("AST submodule was not found. Try to run manually next command using a terminal: git submodule update --init --recursive --remote")
        exit(1)
finally:
    InstallAST()