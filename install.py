try:
    from .dependencies.AST.install import *
except ImportError:
    try:
        from dependencies.AST.install import *
    except ImportError:
        print("Impossible to find .py library by the next path: dependencies/AST/install.py - try to update git submodules and try again")

if __name__ == '__main__':
    InstallAST()