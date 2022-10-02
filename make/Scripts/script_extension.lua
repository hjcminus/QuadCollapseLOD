--------------------------------------------------------------------------------
-- customize some functions --
--------------------------------------------------------------------------------

-- VS Check
is_vs2019 = true
if string.find(_ACTION, "vs2019") == nil then
  is_vs2019 = false
end

-- Linux Check
is_linux = false
if string.find(_ACTION, "gmake2") ~= nil then
  is_linux = true
end

if is_vs2019 then

	require ("vstudio")

	local p = premake
	local m = p.vstudio.vc2010

	-- support ATL server
	p.api.addAllowed("flags", "ATL")
	p.api.addAllowed("flags", "Admin")
	p.api.addAllowed("flags", "DisableSpectreMitigation")

	p.api.register {
		name = "MidlHeader",
		scope = "config",
		kind = "string",
		tokens = true,
	}

	p.api.register {
		name = "MidlInterfaceIdentifier",
		scope = "config",
		kind = "string",
		tokens = true,
	}

	p.api.register {
		name = "MidlProxy",
		scope = "config",
		kind = "string",
		tokens = true,
	}

	p.api.register {
		name = "MidlTypeLibrary",
		scope = "config",
		kind = "string",
		tokens = true,
	}

	-- provide Midl writer
	m.Midl = function(cfg)
		if cfg.MidlHeader and cfg.MidlInterfaceIdentifier and cfg.MidlProxy and cfg.MidlTypeLibrary then
			p.push('<Midl>')
			p.w("<MkTypLibCompatible>false</MkTypLibCompatible>")
			p.w('<HeaderFileName>%s</HeaderFileName>', cfg.MidlHeader)
			p.w('<InterfaceIdentifierFileName>%s</InterfaceIdentifierFileName>', cfg.MidlInterfaceIdentifier)
			p.w('<ProxyFileName>%s</ProxyFileName>', cfg.MidlProxy)
			p.w("<GenerateStublessProxies>true</GenerateStublessProxies>")
			p.w('<TypeLibraryName>%s</TypeLibraryName>', cfg.MidlTypeLibrary)
			p.w("<DllDataFileName></DllDataFileName>")
			p.pop('</Midl>')
		end
	end

	-- override default implementation
	p.override(m, "keyword", function(base, prj)
		for cfg in p.project.eachconfig(prj) do
			if cfg.flags.ATL then
				m.element("Keyword", nil, "AtlProj")
				return
			end
			
			if cfg.flags.MFC then
				m.element("Keyword", nil, "MFCProj")
				return
			end
		end
		base(prj)
	end)

	function m.disableSpectreMitigation(cfg)
		if cfg.flags.DisableSpectreMitigation then
			m.element("SpectreMitigation", nil, "false")
		end
	end

	p.override(m.elements, "configurationProperties", function(base, cfg)
		if cfg.kind == p.UTILITY then
			return {
				m.configurationType,
				m.platformToolset,
				m.disableSpectreMitigation,
			}
		else
			return {
				m.configurationType,
				m.useDebugLibraries,
				m.useOfMfc,
				m.useOfAtl,
				m.clrSupport,
				m.characterSet,
				m.platformToolset,
				m.disableSpectreMitigation,
				m.wholeProgramOptimization,
				m.nmakeOutDirs,
				m.windowsSDKDesktopARMSupport,
			}
		end
	end)

	p.override(m.elements, "itemDefinitionGroup", function(base, cfg)
		if cfg.kind == p.UTILITY then
			return {
				m.ruleVars,
				m.buildEvents,
				m.buildLog,
			}
		else
			return {
				m.clCompile,
				m.Midl,
				m.resourceCompile,
				m.linker,
				m.manifest,
				m.buildEvents,
				m.imageXex,
				m.deploy,
				m.ruleVars,
				m.buildLog,
			}
		end
	end)

	-- override default link implementation
	function m.UACExecutionLevel(cfg)
		if cfg.flags.Admin then
			m.element("UACExecutionLevel", nil, "RequireAdministrator")
		end
	end
		
	p.override(m.elements, "link", function(base, cfg, explicit)
		if cfg.kind == p.STATICLIB then
			return {
				m.subSystem,
				m.fullProgramDatabaseFile,
				m.generateDebugInformation,
				m.optimizeReferences,
			}
		else
			return {
				m.subSystem,
				m.fullProgramDatabaseFile,
				m.generateDebugInformation,
				m.optimizeReferences,
				m.additionalDependencies,
				m.additionalLibraryDirectories,
				m.importLibrary,
				m.entryPointSymbol,
				m.generateMapFile,
				m.moduleDefinitionFile,
				m.treatLinkerWarningAsErrors,
				m.ignoreDefaultLibraries,
				m.largeAddressAware,
				m.targetMachine,
				m.additionalLinkOptions,
				m.programDatabaseFile,
				m.UACExecutionLevel,
			}
		end
	end)

end


