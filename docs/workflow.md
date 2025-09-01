# CodeCAD Workflow

## Create project

```
> mkdir my_project && cd my_project
> ccad init
```

This creates the initial project file:

- `project.json` contains meta information and configurations

## Create part

The next step is to create a new part which contains shape information.

```
> ccad parts add name <part name>
```

This creates a new part file with a dummy shape in `parts/part_name.lua` and adds the part to the `project.json` file.

## Start live viewer

Before getting started with modeling, you can start the live viewer to see the modelling progress.

```
> ccad live
```

This should show you the project rendered in 3D.

## Start modelling

Open up the part file, and start to model the first part. Every time the file gets saved, `ccad` updates the viewer. In case of errors, the user gets informed to fix the problem.

## Write the result

Once finished with modeling, you can simply generate the output files either as `.stl` or as `.step` files with the following command.

```
> ccad build
```

## Setup LSP

If you want to work with LSP support make sure to generate the according files with the following command.

```
> ccad lsp
```
